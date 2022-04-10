#pragma once

#include "Falcor.h"
#include "../BaseRasterPass.h"
#include "../common.h"
#include "../SettingsObject.h"

class RTRad;

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

private:
    CITPass(const Scene::SharedPtr& pScene);
};

