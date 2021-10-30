#include "CITPass.h"
#include "RTRad.h"

using namespace Falcor;

CITPass::CITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
    : BaseGraphicsPass(progDesc, programDefines), mpScene(pScene)
{
    assert(pScene);
}

CITPass::SharedPtr CITPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a CITPass object without a scene");

    Program::DefineList dl = Program::DefineList();
    dl.add(pScene->getSceneDefines());

    Falcor::Program::Desc desc;

    desc.addShaderLibrary("Samples/RTRad/CITP.vs.hlsl");
    desc.vsEntry("main");

    desc.addShaderLibrary("Samples/RTRad/CITP.gs.hlsl");
    desc.gsEntry("gmain");

    desc.addShaderLibrary("Samples/RTRad/CITP.ps.hlsl");
    desc.psEntry("pmain");

    return SharedPtr(new CITPass(pScene, desc, dl));
}

void CITPass::renderScene(RenderContext* pContext, const Texture::SharedPtr posTex, const Texture::SharedPtr nrmTex, const Texture::SharedPtr arfTex, const Texture::SharedPtr li0Tex, const Texture::SharedPtr li1Tex)
{
    // Create FBO
    std::vector<Texture::SharedPtr> tfbo;
    tfbo.push_back(posTex);
    tfbo.push_back(nrmTex);
    tfbo.push_back(arfTex);
    tfbo.push_back(li0Tex);
    tfbo.push_back(li1Tex);
    Fbo::SharedPtr fbo = Fbo::create(tfbo);

    mpState->setFbo(fbo);
    mpScene->rasterize(pContext, mpState.get(), mpVars.get());
}
