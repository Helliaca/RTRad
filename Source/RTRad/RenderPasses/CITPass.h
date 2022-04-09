#pragma once

#include "Falcor.h"
#include "../RR_BaseRenderPass.h"
#include "../common.h"

class RTRad;

using namespace Falcor;

class CITPass : public RR_BaseRenderPass, public std::enable_shared_from_this<CITPass>
{
public:
    using SharedPtr = std::shared_ptr<CITPass>;

    static SharedPtr create(const Scene::SharedPtr& pScene);

    void renderScene(RenderContext* pContext, const TextureGroup tg);
private:
    CITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
};

