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

static int c = 0;
static int passNum = 0;

void RTLightmapPass::setPerFrameVars(const TextureGroup textureGroup)
{
    PROFILE("setPerFrameVars");
    rtVars->setTexture("pos", textureGroup.posTex);
    rtVars->setTexture("nrm", textureGroup.nrmTex);
    rtVars->setTexture("arf", textureGroup.arfTex);
    rtVars->setTexture("mat", textureGroup.matTex);
    rtVars->setTexture("lig", textureGroup.lgiTex);
    rtVars->setTexture("lig2", textureGroup.lgoTex);
    rtVars->setTexture("voxTex", textureGroup.voxTex);
    rtVars["PerFrameCB"]["row_offset"] = row_offset;
    rtVars["PerFrameCB"]["sampling_res"] = settings.sampling_res;
    rtVars["PerFrameCB"]["posOffset"] = scene->getSceneBounds().minPoint;
    rtVars["PerFrameCB"]["randomizeSamples"] = settings.randomizeSample;
    rtVars["PerFrameCB"]["texRes"] = textureGroup.lgiTex.get()->getWidth();
    rtVars["PerFrameCB"]["passNum"] = passNum;
    rtVars["PerFrameCB"]["useVisCache"] = settings.useVisCache;

    if (settings.useVisCache) {
        rtVars["vis"] = textureGroup.visBuf;
    }
}

void RTLightmapPass::render(RenderContext* pContext, const TextureGroup textureGroup)
{
    PROFILE("renderRT");

    assert(scene);
    if (is_set(scene->getUpdates(), Scene::UpdateFlags::GeometryChanged))
    {
        throw std::runtime_error("This sample does not support scene geometry changes. Aborting.");
    }

    setPerFrameVars(textureGroup);

    //int rays_per_texel = ligTex->getWidth() * ligTex->getHeight(); //TODO: we do not consider the LOD-cheat here
    //int texels_amount = max_rays_per_batch / rays_per_texel;

    int xres = textureGroup.lgoTex->getWidth(), yres = textureGroup.lgoTex->getHeight();

    //batch_counter++;
    //row_offset = 

    scene->raytrace(pContext, rtProgram.get(), rtVars, uint3(settings.texPerBatch*xres, yres, 1));

    row_offset += (int)(settings.texPerBatch * xres);
}

bool RTLightmapPass::runBatch(RenderContext* pContext, const TextureGroup textureGroup)
{
    int xres = textureGroup.lgoTex->getWidth(), yres = textureGroup.lgoTex->getHeight();

    render(pContext, textureGroup);

    if (row_offset >= xres) {
        passNum++;
        row_offset = 0;

        //std::vector<Texture::SharedPtr> tfbo;
        //tfbo.push_back(textureGroup.lgoTex);
        //Fbo::SharedPtr fbo = Fbo::create(tfbo);

        //fsp->getVars()->setTexture("ligTex", textureGroup.lgoTex);

        //fsp->execute(pContext, fbo, true);


        return true;
    }
    else {
        return false;
    }
}

void RTLightmapPass::onGuiRender(Gui* pGui, Gui::Window w)
{
    Falcor::Gui::DropdownList integralModes;

    //uint32_t sres = rtlSettings.sampling_res;
    integralModes.push_back({ (int)RTPassIntegral::AREA, "Area" });
    integralModes.push_back({ (int)RTPassIntegral::HEMISPHERIC, "Hemispheric" });

    //w.dropdown("Integral Mode", integralModes, sett);
    
}

RTLightmapPass::RTLightmapPass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable)
    : BaseRaytracePass(pScene, programDesc, bindingTable)
{
    row_offset = 0;

    fsp = FullScreenPass::create(SHADERS_FOLDER"/FixSeams.ps.hlsl", scene->getSceneDefines());

    settings.randomizeSample = false;
    settings.sampling_res = 1;
    settings.texPerBatch = 0.5f;
    settings.useVisCache = false;
    settings.integral = RTPassIntegral::AREA;
}
