#pragma once
#include "Falcor.h"

using namespace Falcor;

class VITPass : public BaseGraphicsPass, public std::enable_shared_from_this<VITPass>
{
public:
    using SharedPtr = std::shared_ptr<VITPass>;

    static SharedPtr create(const Scene::SharedPtr& pScene);

    void renderScene(RenderContext* pContext, const Texture::SharedPtr disTex, const Falcor::Fbo::SharedPtr outputFbo, const bool applyToModel, const bool showTexRes=false, float4 interp_min=float4(0.0f), float4 interp_max=float4(1.0f));

    const Scene::SharedPtr& getScene() const { return mpScene; }
private:
    VITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
    Scene::SharedPtr mpScene;
};

