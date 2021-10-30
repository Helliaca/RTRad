#include "RTLightmapPass.h"

using namespace Falcor;

RTLightmapPass::SharedPtr RTLightmapPass::create()
{
    return SharedPtr(new RTLightmapPass());
}

void RTLightmapPass::load(const Scene::SharedPtr mpScene)
{
    // We'll now create a raytracing program. To do that we need to setup two things:
    // - A program description (RtProgram::Desc). This holds all shader entry points, compiler flags, macro defintions, etc.
    // - A binding table (RtBindingTable). This maps shaders to geometries in the scene, and sets the ray generation and miss shaders.
    //
    // After setting up these, we can create the RtProgram and associated RtProgramVars that holds the variable/resource bindings.
    // The RtProgram can be reused for different scenes, but RtProgramVars needs to binding table which is Scene-specific and
    // needs to be re-created when switching scene. In this example, we re-create both the program and vars when a scene is loaded.

    RtProgram::Desc rtProgDesc;
    rtProgDesc.addShaderLibrary("Samples/RTRad/rayTracedLightMap.rt.hlsl");
    rtProgDesc.addDefines(mpScene->getSceneDefines());
    rtProgDesc.setMaxTraceRecursionDepth(1); // 1 for calling TraceRay from RayGen, 1 for calling it from the primary-ray ClosestHit shader for reflections, 1 for reflection ray tracing a shadow ray
    rtProgDesc.setMaxPayloadSize(24); // The largest ray payload struct (PrimaryRayData) is 24 bytes. The payload size should be set as small as possible for maximum performance.

    RtBindingTable::SharedPtr sbt = RtBindingTable::create(1, 1, mpScene->getGeometryCount());

    sbt->setRayGen(rtProgDesc.addRayGen("rayGen"));
    sbt->setMiss(0, rtProgDesc.addMiss("primaryMiss"));

    //sbt->setMiss(1, rtProgDesc.addMiss("shadowMiss"));

    //auto primary = rtProgDesc.addHitGroup("primaryClosestHit", "primaryAnyHit");
    //auto shadow = rtProgDesc.addHitGroup("", "shadowAnyHit");

    //sbt->setHitGroupByType(0, mpScene, Scene::GeometryType::TriangleMesh, primary);
    //sbt->setHitGroupByType(1, mpScene, Scene::GeometryType::TriangleMesh, shadow);

    mpRaytraceProgram = RtProgram::create(rtProgDesc);
    mpRtVars = RtProgramVars::create(mpRaytraceProgram, sbt);

    this->mpScene = mpScene;
}

void RTLightmapPass::setPerFrameVars(const Texture::SharedPtr posTex, const Texture::SharedPtr nrmTex, const Texture::SharedPtr ligTex, const Texture::SharedPtr outTex)
{
    PROFILE("setPerFrameVars");
    mpRtVars->setTexture("pos", posTex);
    mpRtVars->setTexture("nrm", nrmTex);
    mpRtVars->setTexture("lig", ligTex);
    mpRtVars->setTexture("lig2", outTex);
}

void RTLightmapPass::renderRT(RenderContext* pContext, const Fbo* pTargetFbo, const Camera::SharedPtr mpCamera, const Texture::SharedPtr posTex, const Texture::SharedPtr nrmTex, const Texture::SharedPtr ligTex, const Texture::SharedPtr outTex)
{
    PROFILE("renderRT");

    assert(mpScene);
    if (is_set(mpScene->getUpdates(), Scene::UpdateFlags::GeometryChanged))
    {
        throw std::runtime_error("This sample does not support scene geometry changes. Aborting.");
    }

    setPerFrameVars(posTex, nrmTex, ligTex, outTex);

    mpScene->raytrace(pContext, mpRaytraceProgram.get(), mpRtVars, uint3(ligTex->getWidth(), ligTex->getHeight(), 1));
}
