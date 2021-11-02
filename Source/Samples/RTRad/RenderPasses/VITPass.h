#pragma once
#include "Falcor.h"

using namespace Falcor;

class VITPass : public BaseGraphicsPass, public std::enable_shared_from_this<VITPass>
{
public:
    using SharedPtr = std::shared_ptr<VITPass>;

    static SharedPtr create(const Scene::SharedPtr& pScene);

    void renderScene(RenderContext* pContext, const Texture::SharedPtr disTex, const Falcor::Fbo::SharedPtr outputFbo, const bool applyToModel, const bool treatAsMatIDs=false, const bool showTexRes=false);

    const Scene::SharedPtr& getScene() const { return mpScene; }
private:
    VITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
    Scene::SharedPtr mpScene;
};

