#include "RTLightmapPass.h"

using namespace Falcor;

RTLightmapPass::SharedPtr RTLightmapPass::create(const Scene::SharedPtr& mpScene)
{
    // We'll now create a raytracing program. To do that we need to setup two things:
    // - A program description (RtProgram::Desc). This holds all shader entry points, compiler flags, macro defintions, etc.
    // - A binding table (RtBindingTable). This maps shaders to geometries in the scene, and sets the ray generation and miss shaders.
    //
    // After setting up these, we can create the RtProgram and associated RtProgramVars that holds the variable/resource bindings.
    // The RtProgram can be reused for different scenes, but RtProgramVars needs to binding table which is Scene-specific and
    // needs to be re-created when switching scene. In this example, we re-create both the program and vars when a scene is loaded.

    RtProgram::Desc rtProgDesc;

#if HEMISPHERIC_SAMPLING
    rtProgDesc.addShaderLibrary(RTLPASS_DIR_SHADERS"/rayTracedLightMap2.rt.hlsl");
#else
    rtProgDesc.addShaderLibrary(RTLPASS_DIR_SHADERS"/rayTracedLightMap.rt.hlsl");
#endif

    rtProgDesc.addDefines(mpScene->getSceneDefines());
    rtProgDesc.setMaxTraceRecursionDepth(1); // 1 for calling TraceRay from RayGen, 1 for calling it from the primary-ray ClosestHit shader for reflections, 1 for reflection ray tracing a shadow ray
    rtProgDesc.setMaxPayloadSize(24); // The largest ray payload struct (PrimaryRayData) is 24 bytes. The payload size should be set as small as possible for maximum performance.

    RtBindingTable::SharedPtr sbt = RtBindingTable::create(1, 1, mpScene->getGeometryCount());

    sbt->setRayGen(rtProgDesc.addRayGen("rayGen"));
    sbt->setMiss(0, rtProgDesc.addMiss("primaryMiss"));

#if HEMISPHERIC_SAMPLING
    // This code only if we run hit-shaders
    auto primary = rtProgDesc.addHitGroup("primaryClosestHit", "primaryAnyHit");
    sbt->setHitGroupByType(0, mpScene, Scene::GeometryType::TriangleMesh, primary);
#endif

    RTLightmapPass* pass = new RTLightmapPass(mpScene, rtProgDesc, sbt);

    return SharedPtr(pass);
}

void RTLightmapPass::setPerFrameVars(const TextureGroup* textureGroup)
{
    PROFILE("setPerFrameVars");
    rtVars->setTexture("pos", textureGroup->posTex);
    rtVars->setTexture("nrm", textureGroup->nrmTex);
    rtVars->setTexture("arf", textureGroup->arfTex);
    rtVars->setTexture("mat", textureGroup->matTex);
    rtVars->setTexture("lig", textureGroup->lgiTex);
    rtVars->setTexture("lig2", textureGroup->lgoTex);
    rtVars->setTexture("voxTex", textureGroup->voxTex);
    rtVars["PerFrameCB"]["row_offset"] = settings.row_offset;
    rtVars["PerFrameCB"]["sampling_res"] = settings.sampling_res;
    rtVars["PerFrameCB"]["posOffset"] = scene->getSceneBounds().minPoint;
    rtVars["PerFrameCB"]["randomizeSamples"] = settings.randomizeSample;
    rtVars["PerFrameCB"]["texRes"] = textureGroup->lgiTex.get()->getWidth();
    rtVars["PerFrameCB"]["passNum"] = settings.passNum;
    rtVars["PerFrameCB"]["useVisCache"] = settings.useVisCache;

    if (settings.useVisCache) {
        rtVars["vis"] = textureGroup->visBuf;
    }
}

void RTLightmapPass::render(RenderContext* pContext, const TextureGroup* textureGroup)
{
    PROFILE("renderRT");

    // Scene changes
    assert(scene);
    if (is_set(scene->getUpdates(), Scene::UpdateFlags::GeometryChanged))
    {
        throw std::runtime_error("This sample does not support scene geometry changes. Aborting.");
    }

    // Set vars
    setPerFrameVars(textureGroup);

    // Get resolution
    int xres = textureGroup->lgoTex->getWidth(), yres = textureGroup->lgoTex->getHeight();

    // Run this batch
    scene->raytrace(pContext, rtProgram.get(), rtVars, uint3(settings.texPerBatch*xres, yres, 1));

    // set row_offset
    settings.row_offset += (int)(settings.texPerBatch * xres);

    // batch finished ?
    if (settings.row_offset >= xres) {
        settings.row_offset = 0;
        settings.passNum++;
        settings.batchComplete = true;
    }
    else
    {
        settings.batchComplete = false;
    }
}

void RTLightmapPass::onRenderGui(Gui* Gui, Gui::Window* win)
{
    win->text("RTLPass Settings");

    win->checkbox("Randomize Sample", settings.randomizeSample);

    win->checkbox("Use VisCache", settings.useVisCache);

    Falcor::Gui::DropdownList sreslst;
    uint32_t sres = settings.sampling_res;
    sreslst.push_back({ 1, "1x1" });
    sreslst.push_back({ 2, "2x2" });
    sreslst.push_back({ 4, "4x4" });
    sreslst.push_back({ 8, "8x8" });
    sreslst.push_back({ 16, "16x16" });
    win->dropdown("Sampling Res", sreslst, sres);
    settings.sampling_res = sres;

    win->slider("Tex per Batch", settings.texPerBatch, 0.01f, 1.0f);
}

RTLightmapPass::RTLightmapPass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable)
    : BaseRaytracePass(pScene, programDesc, bindingTable)
{
    settings = RTLPassSettings::RTLPassSettings();
}
