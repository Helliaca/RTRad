#pragma once

#include "Falcor.h"
#include "../BaseRenderPass.h"
#include "../common.h"
#include "../SettingsObject.h"

class RTRad;

using namespace Falcor;

struct CITPassSettings : public RR::BaseSettings {
    bool enabled;
};


class CITPass : public RR::BaseRenderPass, public std::enable_shared_from_this<CITPass>, public RR::SettingsObject<CITPassSettings>
{
public:
    using SharedPtr = std::shared_ptr<CITPass>;

    static SharedPtr create(const Scene::SharedPtr& pScene);

    void renderScene(RenderContext* pContext, const TextureGroup tg);
private:
    CITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
};

