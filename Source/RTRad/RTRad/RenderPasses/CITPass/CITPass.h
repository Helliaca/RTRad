#pragma once

#include "Falcor.h"
#include <RTRad/Core/BaseRasterPass.h>
#include <RTRad/Core/common.h>
#include <RTRad/Core/SettingsObject.h>

using namespace Falcor;

struct CITPassSettings : public RR::BaseSettings {
    // No settings for CITPass yet
};


class CITPass : public RR::BaseRasterPass, public std::enable_shared_from_this<CITPass>, public RR::SettingsObject<CITPassSettings>
{
public:
    // SmartPointer
    using SharedPtr = std::shared_ptr<CITPass>;
    static SharedPtr create(const Scene::SharedPtr& pScene);

    // Render
    void render(RenderContext* pContext, const TextureGroup tg) override;
    void setPerFrameVars(const TextureGroup textureGroup) override;

    void onRenderGui(Gui* Gui, Gui::Window* win) override;

private:
    CITPass(const Scene::SharedPtr& pScene);
};

