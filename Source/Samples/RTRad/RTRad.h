#pragma once
#include "Falcor.h"
#include "CITPass.h"
#include "VITPass.h"
#include "RTLightmapPass.h"

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

    Scene::SharedPtr mpScene;

private:
    CITPass::SharedPtr mpRasterPass;
    VITPass::SharedPtr vitPass;
    RTLightmapPass::SharedPtr rtlPass;
    //Scene::SharedPtr mpScene;

    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    Camera::SharedPtr mpCamera;

    bool mRayTrace = false;
    bool mUseDOF = false;
    RtProgramVars::SharedPtr mpRtVars;
    Texture::SharedPtr mpRtOut;

    bool mApplyToModel = true;
    uint32_t outputTex = 0;
    bool mResetInputTextures = true;

    bool makePass = false;

    int passNum = 0;

    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr arfTex;
    Texture::SharedPtr li0Tex;
    Texture::SharedPtr li1Tex;

    uint32_t mSampleIndex = 0xdeadbeef;

    void loadScene(const std::string& filename, const Fbo* pTargetFbo);
};
