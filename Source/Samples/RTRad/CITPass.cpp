#include "CITPass.h"

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
    desc.addShaderLibrary("Samples/RTRad/CITP.ps.hlsl");
    desc.psEntry("pmain");

    return SharedPtr(new CITPass(pScene, desc, dl));
}

void CITPass::renderScene(RenderContext* pContext, const Fbo::SharedPtr& pDstFbo)
{
    mpState->setFbo(pDstFbo);
    mpScene->rasterize(pContext, mpState.get(), mpVars.get());
}
