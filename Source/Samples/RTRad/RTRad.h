#pragma once
#include "Falcor.h"
#include "CITPass.h"

using namespace Falcor;

class RTRad : public IRenderer
{
public:
    void onLoad(RenderContext* pRenderContext) override;
    void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onShutdown() override;
    void onResizeSwapChain(uint32_t width, uint32_t height) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;
    void onGuiRender(Gui* pGui) override;

private:
    CITPass::SharedPtr mpRasterPass;
    Scene::SharedPtr mpScene;

    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    Camera::SharedPtr mpCamera;

    bool mRayTrace = false;
    bool mUseDOF = false;
    RtProgramVars::SharedPtr mpRtVars;
    Texture::SharedPtr mpRtOut;

    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr li0Tex;
    Texture::SharedPtr li1Tex;

    uint32_t mSampleIndex = 0xdeadbeef;

    void setPerFrameVars(const Fbo* pTargetFbo);
    void renderRT(RenderContext* pContext, const Fbo* pTargetFbo);
    void loadScene(const std::string& filename, const Fbo* pTargetFbo);
};
