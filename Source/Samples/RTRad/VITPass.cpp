#include "VITPass.h"

using namespace Falcor;

VITPass::VITPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
    : BaseGraphicsPass(progDesc, programDefines), mpScene(pScene)
{
    assert(pScene);
}

VITPass::SharedPtr VITPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a VITPass object without a scene");

    Program::DefineList dl = Program::DefineList();
    dl.add(pScene->getSceneDefines());

    Falcor::Program::Desc desc;
    desc.addShaderLibrary("Samples/RTRad/VITP.vs.hlsl");
    desc.vsEntry("main");
    desc.addShaderLibrary("Samples/RTRad/VITP.ps.hlsl");
    desc.psEntry("pmain");

    return SharedPtr(new VITPass(pScene, desc, dl));
}

void VITPass::renderScene(RenderContext* pContext, const Texture::SharedPtr disTex, const Falcor::Fbo::SharedPtr outputFbo, const bool applyToModel, const bool treatAsMatIDs)
{
    Falcor::GraphicsVars::SharedPtr vars = Falcor::GraphicsVars::create(this->getProgram().get());
    vars->setTexture("disTex", disTex);
    vars["PerFrameCB"]["applyToModel"] = applyToModel;
    vars["PerFrameCB2"]["treatAsMatIDs"] = treatAsMatIDs;
    this->setVars(vars);

    mpState->setFbo(outputFbo);
    mpScene->rasterize(pContext, mpState.get(), mpVars.get());
}
