#include "RTLightmapPass.h"

using namespace Falcor;

RTLightmapPass::SharedPtr RTLightmapPass::create(const Scene::SharedPtr& mpScene)
{
    RTLightmapPass* pass = new RTLightmapPass();
    pass->load(mpScene);
    return SharedPtr(pass);
}

void RTLightmapPass::load(const Scene::SharedPtr& mpScene)
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
    rtProgDesc.addShaderLibrary(SHADERS_FOLDER"/rayTracedLightMap2.rt.hlsl");
#else
    rtProgDesc.addShaderLibrary(SHADERS_FOLDER"/rayTracedLightMap.rt.hlsl");
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

    mpRaytraceProgram = RtProgram::create(rtProgDesc);
    mpRtVars = RtProgramVars::create(mpRaytraceProgram, sbt);

    this->mpScene = mpScene;

    row_offset = 0;

    fsp = FullScreenPass::create(SHADERS_FOLDER"/FixSeams.ps.hlsl", mpScene->getSceneDefines());
}

static int c = 0;

void RTLightmapPass::setPerFrameVars(const TextureGroup textureGroup, const RTLightmapPassSettings settings)
{
    PROFILE("setPerFrameVars");
    mpRtVars->setTexture("pos", textureGroup.posTex);
    mpRtVars->setTexture("nrm", textureGroup.nrmTex);
    mpRtVars->setTexture("arf", textureGroup.arfTex);
    mpRtVars->setTexture("mat", textureGroup.matTex);
    mpRtVars->setTexture("lig", textureGroup.lgiTex);
    mpRtVars->setTexture("lig2", textureGroup.lgoTex);
    mpRtVars["PerFrameCB"]["row_offset"] = row_offset;
    mpRtVars["PerFrameCB"]["sampling_res"] = settings.sampling_res;
    mpRtVars["PerFrameCB"]["posOffset"] = mpScene->getSceneBounds().minPoint;
    mpRtVars["PerFrameCB"]["randomizeSamples"] = settings.randomizeSample;
    mpRtVars["PerFrameCB"]["passNum"] = c++; //NOTE: This is *NOT* the true passnum. (Its the batchnumber)
}

void RTLightmapPass::renderRT(RenderContext* pContext, const TextureGroup textureGroup, const RTLightmapPassSettings settings)
{
    PROFILE("renderRT");

    assert(mpScene);
    if (is_set(mpScene->getUpdates(), Scene::UpdateFlags::GeometryChanged))
    {
        throw std::runtime_error("This sample does not support scene geometry changes. Aborting.");
    }

    setPerFrameVars(textureGroup, settings);

    //int rays_per_texel = ligTex->getWidth() * ligTex->getHeight(); //TODO: we do not consider the LOD-cheat here
    //int texels_amount = max_rays_per_batch / rays_per_texel;

    int xres = textureGroup.lgoTex->getWidth(), yres = textureGroup.lgoTex->getHeight();

    //batch_counter++;
    //row_offset = 

    mpScene->raytrace(pContext, mpRaytraceProgram.get(), mpRtVars, uint3(settings.texPerBatch*xres, yres, 1));

    row_offset += (int)(settings.texPerBatch * xres);
}

bool RTLightmapPass::runBatch(RenderContext* pContext, const TextureGroup textureGroup, const RTLightmapPassSettings settings)
{
    int xres = textureGroup.lgoTex->getWidth(), yres = textureGroup.lgoTex->getHeight();

    renderRT(pContext, textureGroup, settings);

    if (row_offset >= xres) {
        row_offset = 0;

        //fsp->execute(pContext, textureGroup.lgoTex->getF)
        std::vector<Texture::SharedPtr> tfbo;
        tfbo.push_back(textureGroup.lgoTex);
        Fbo::SharedPtr fbo = Fbo::create(tfbo);

        fsp->getVars()->setTexture("ligTex", textureGroup.lgoTex);

        fsp->execute(pContext, fbo, true);


        return true;
    }
    else {
        return false;
    }
}
