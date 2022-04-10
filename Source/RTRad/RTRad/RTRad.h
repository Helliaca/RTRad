#pragma once
#include "Falcor.h"

#include <RTRad/Core/common.h>

#include CITPASS_H

#include "RenderPasses/VITPass.h"
#include "RenderPasses/RTLightmapPass.h"
#include "RenderPasses/CVMPass.h"

using namespace Falcor;

class RTRad : public IRenderer
{
public:
    // GUI Controls
    void onGuiRender(Gui* pGui) override;

    // Scene loading and initialization
    void onLoad(RenderContext* pRenderContext) override;
    void loadScene(const std::string& filename);

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

    CVMPass::SharedPtr cvmPass;

    // Texture group for the pipeline
    TextureGroup textureGroup;

    // Scene data
    Scene::SharedPtr mpScene;
    Camera::SharedPtr mpCamera;

    // Flow-Control variables
    uint32_t mOutputTex = 5;
    bool mResetInputTextures = true;
    bool mResetVoxelMap = true;
    bool mMakePass = false;
    bool mMakeBatch = false;
    uint32_t mTextureRes = 64;

    // Output/Measurement variables
    std::string mOutputString = " -- Empty -- ";
    double mAccTime = 0;
};