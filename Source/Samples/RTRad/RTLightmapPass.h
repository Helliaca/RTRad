#pragma once

#include "Falcor.h"

using namespace Falcor;

class RTLightmapPass : public std::enable_shared_from_this<RTLightmapPass>
{
public:
    using SharedPtr = std::shared_ptr<RTLightmapPass>;

    static SharedPtr create();

    void load(const Scene::SharedPtr mpScene);
    void setPerFrameVars(const Texture::SharedPtr posTex, const Texture::SharedPtr nrmTex, const Texture::SharedPtr ligTex, const Texture::SharedPtr outTex);
    void renderRT(RenderContext* pContext, const Fbo* pTargetFbo, const Camera::SharedPtr mpCamera, const Texture::SharedPtr posTex, const Texture::SharedPtr nrmTex, const Texture::SharedPtr ligTex, const Texture::SharedPtr outTex);

private:
    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    RtProgramVars::SharedPtr mpRtVars;

    Texture::SharedPtr mpRtOut;

    Scene::SharedPtr mpScene;
};

