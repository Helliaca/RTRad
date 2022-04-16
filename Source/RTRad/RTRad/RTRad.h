#pragma once
#include "Falcor.h"

#include <RTRad/Core/common.h>

#include CITPASS_H
#include VITPASS_H
#include CVMPASS_H
#include RTLPASS_H

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

    RTLightmapPass::SharedPtr rtlPass;

    CVMPass::SharedPtr cvmPass;

    // Texture group for the pipeline
    TextureGroup textureGroup;

    // Scene data
    Scene::SharedPtr mpScene;
    Camera::SharedPtr mpCamera;

    // Flow-Control variables
    bool mResetInputTextures = true;
    bool mMakePass = false;
    bool mMakeBatch = false;
    uint32_t mTextureRes = 64;
    uint32_t mVoxelRes = 64;

    // Output/Measurement variables
    std::string mOutputString = " -- Empty -- ";
    double mAccTime = 0;
};
