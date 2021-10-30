#pragma once

#include "Falcor.h"

using namespace Falcor;

class RTLightmapPass : public std::enable_shared_from_this<RTLightmapPass>
{
public:
    using SharedPtr = std::shared_ptr<RTLightmapPass>;

    static SharedPtr create();

    void load(const Scene::SharedPtr mpScene);
    void setPerFrameVars(const Fbo* pTargetFbo, const Camera::SharedPtr mpCamera);
    void renderRT(RenderContext* pContext, const Fbo* pTargetFbo, const Camera::SharedPtr mpCamera);

private:
    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    RtProgramVars::SharedPtr mpRtVars;

    Texture::SharedPtr mpRtOut;

    Scene::SharedPtr mpScene;
};

