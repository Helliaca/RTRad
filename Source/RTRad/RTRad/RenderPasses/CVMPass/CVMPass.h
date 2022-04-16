#pragma once

#include "Falcor.h"
#include <RTRad/Core/BaseRasterPass.h>
#include <RTRad/Core/common.h>
#include <RTRad/Core/SettingsObject.h>

using namespace Falcor;

struct CVMPassSettings : RR::BaseSettings {

};

class CVMPass : public RR::BaseRasterPass, public RR::SettingsObject<CVMPassSettings>, public std::enable_shared_from_this<CVMPass>
{
public:
    // Pointer
    using SharedPtr = std::shared_ptr<CVMPass>;
    static SharedPtr create(const Scene::SharedPtr& pScene);

    // Render
    void render(RenderContext* pContext, const TextureGroup* tg) override;
    void setPerFrameVars(const TextureGroup* textureGroup) override;

    void onRenderGui(Gui* Gui, Gui::Window* win) override;

private:
    CVMPass(const Scene::SharedPtr& pScene);

    // Fbo
    bool CurrentFboIsValid(const TextureGroup* tg);
    void MakeFbo(const TextureGroup* tg);
    Fbo::SharedPtr voxFbo;
};

