#pragma once

#include "Falcor.h"
#include <RTRad/Core/BaseRasterPass.h>
#include <RTRad/Core/common.h>
#include <RTRad/Core/SettingsObject.h>

using namespace Falcor;

struct VITPassSettings : RR::BaseSettings {
    bool maskGeometry;
    bool applyToModel;
    bool showTexRes;
    bool showVoxelMap;
    float4 interp_min;
    float4 interp_max;
    int mipmapLevel;
    uint32_t outputTexture;

    VITPassSettings() {
        maskGeometry = true;
        applyToModel = true;
        showTexRes = false;
        showVoxelMap = false;
        interp_min = float4(0.0f);
        interp_max = float4(1.0f);
        mipmapLevel = 0;
        outputTexture = 0;
    }
};

class VITPass : public RR::BaseRasterPass, public RR::SettingsObject<VITPassSettings>, public std::enable_shared_from_this<VITPass>
{
public:
    // Pointer
    using SharedPtr = std::shared_ptr<VITPass>;
    static SharedPtr create(const Scene::SharedPtr& pScene);

    // Render
    void render(RenderContext* pContext, const TextureGroup tg) override;
    void setPerFrameVars(const TextureGroup textureGroup) override;

    // GUI
    void onRenderGui(Gui* Gui, Gui::Window* win) override;

private:
    VITPass(const Scene::SharedPtr& pScene);
    Scene::SharedPtr UVPlaneScene;
};

