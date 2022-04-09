#pragma once

#include "Falcor.h"
#include "RTRadGraphicsPass.h"
#include "../common.h"

class RTRad;

using namespace Falcor;

class CITPass : public RTRadGraphicsPass, public std::enable_shared_from_this<CITPass>
{
public:
    using SharedPtr = std::shared_ptr<CITPass>;

    static SharedPtr create(const Scene::SharedPtr& pScene);

    void renderScene(RenderContext* pContext, const TextureGroup tg);
private:
    CITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
};

