#pragma once
#include "Falcor.h"
#include "RenderPasses/CITPass.h"
#include "RenderPasses/VITPass.h"
#include "RenderPasses/RTLightmapPass.h"

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
    void makeTextures();

    Scene::SharedPtr mpScene;

    TextureGroup textureGroup;

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

    bool makeBatch = false;

    bool showTexRes = false;

    int sampling_res = 1;

    int passNum = 0;

    uint32_t LigRes = 256;

    uint32_t mSampleIndex = 0xdeadbeef;

    float texPerBatch = 0.1f;

    ProfilerUI::UniquePtr mpProfilerUI;

    std::string output = " -- Empty -- ";

    double rttime = 0;

    ProfilerUI::UniquePtr pui;

    void loadScene(const std::string& filename, const Fbo* pTargetFbo);
};