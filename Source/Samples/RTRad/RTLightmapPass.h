#pragma once

#include "Falcor.h"
#include "common.h"

using namespace Falcor;

class RTLightmapPass : public std::enable_shared_from_this<RTLightmapPass>
{
public:
    using SharedPtr = std::shared_ptr<RTLightmapPass>;

    static SharedPtr create();

    void load(const Scene::SharedPtr mpScene);
    void setPerFrameVars(const TextureGroup textureGroup);
    void renderRT(RenderContext* pContext, const Fbo* pTargetFbo, const Camera::SharedPtr mpCamera, const TextureGroup textureGroup);

    bool runBatch(RenderContext* pContext, const Fbo* pTargetFbo, const Camera::SharedPtr mpCamera, const TextureGroup textureGroup);

private:
    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    RtProgramVars::SharedPtr mpRtVars;

    Scene::SharedPtr mpScene;

    uint2 last_index;

    int max_rays_per_batch;
};

