#pragma once
#include "Falcor.h"
#include "RenderPasses/CITPass.h"
#include "RenderPasses/VITPass.h"
#include "RenderPasses/RTLightmapPass.h"

using namespace Falcor;

class RTRad : public IRenderer
{
public:
    // GUI Controls
    void onGuiRender(Gui* pGui) override;

    // Scene loading and initialization
    void onLoad(RenderContext* pRenderContext) override;
    void loadScene(const std::string& filename);
    void makeTextures();

    // Main Render function
    void onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;

    // Callback functions
    void onShutdown() override;
    void onResizeSwapChain(uint32_t width, uint32_t height) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    // RenderPasses
    CITPass::SharedPtr citPass;

    VITPass::SharedPtr vitPass;
    VITPassSettings vitSettings;

    RTLightmapPass::SharedPtr rtlPass;
    RTLightmapPassSettings rtlSettings;

    // Texture group for the pipeline
    TextureGroup textureGroup;

    // Scene data
    Scene::SharedPtr mpScene;
    Camera::SharedPtr mpCamera;

    // Flow-Control variables
    uint32_t mOutputTex = 0;
    bool mResetInputTextures = true;
    bool mMakePass = false;
    bool mMakeBatch = false;
    uint32_t mTextureRes = 32;

    // Output/Measurement variables
    std::string mOutputString = " -- Empty -- ";
    double mAccTime = 0;
};
