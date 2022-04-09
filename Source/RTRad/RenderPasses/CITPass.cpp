#include "CITPass.h"
#include "../RTRad.h"

using namespace Falcor;

CITPass::CITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
    : RTRadGraphicsPass(pScene, progDesc, programDefines)
{
    
}

CITPass::SharedPtr CITPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a CITPass object without a scene");

    Program::DefineList dl = Program::DefineList();
    dl.add(pScene->getSceneDefines());

    Falcor::Program::Desc desc;

    desc.addShaderLibrary(SHADERS_FOLDER"/CITP.vs.hlsl");
    desc.vsEntry("main");

    desc.addShaderLibrary(SHADERS_FOLDER"/CITP.gs.hlsl");
    desc.gsEntry("gmain");

    desc.addShaderLibrary(SHADERS_FOLDER"/CITP.ps.hlsl");
    desc.psEntry("pmain");

    return SharedPtr(new CITPass(pScene, desc, dl));
}

void CITPass::renderScene(RenderContext* pContext, const TextureGroup tg)
{
    // Create FBO
    std::vector<Texture::SharedPtr> tfbo;
    tfbo.push_back(tg.posTex);
    tfbo.push_back(tg.nrmTex);
    tfbo.push_back(tg.arfTex);
    tfbo.push_back(tg.matTex);
    tfbo.push_back(tg.lgiTex);
    tfbo.push_back(tg.lgoTex);
    Fbo::SharedPtr fbo = Fbo::create(tfbo);

    mpVars["PerFrameCB"]["posOffset"] = mpScene->getSceneBounds().minPoint;

    mpState->setFbo(fbo);
    mpScene->rasterize(pContext, mpState.get(), mpVars.get(), RasterizerState::CullMode::None);
}
