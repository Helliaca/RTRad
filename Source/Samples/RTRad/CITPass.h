#pragma once

#include "Falcor.h"

class RTRad;

using namespace Falcor;

class CITPass : public BaseGraphicsPass, public std::enable_shared_from_this<CITPass>
{
public:
    using SharedPtr = std::shared_ptr<CITPass>;

    static SharedPtr create(const Scene::SharedPtr& pScene);

    void renderScene(RenderContext* pContext, const Texture::SharedPtr posTex, const Texture::SharedPtr nrmTex, const Texture::SharedPtr arfTex, const Texture::SharedPtr li0Tex, const Texture::SharedPtr li1Tex);

    const Scene::SharedPtr& getScene() const { return mpScene; }
private:
    CITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
    Scene::SharedPtr mpScene;
};

