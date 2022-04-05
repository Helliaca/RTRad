#pragma once

#include "Falcor.h"
#include "../common.h"

class RTRad;

using namespace Falcor;

class CVMPass : public BaseGraphicsPass, public std::enable_shared_from_this<CVMPass>
{
public:
    using SharedPtr = std::shared_ptr<CVMPass>;

    static SharedPtr create(const Scene::SharedPtr& pScene);

    void renderScene(RenderContext* pContext, const TextureGroup tg);

    const Scene::SharedPtr& getScene() const { return mpScene; }
private:
    CVMPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
    Scene::SharedPtr mpScene;
};

