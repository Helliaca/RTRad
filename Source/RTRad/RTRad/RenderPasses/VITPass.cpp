#include "VITPass.h"
#include <RTRad/Core/common.h>

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
    desc.addShaderLibrary(SHADERS_FOLDER"/VITP.vs.hlsl");
    desc.vsEntry("main");
    desc.addShaderLibrary(SHADERS_FOLDER"/VITP.ps.hlsl");
    desc.psEntry("pmain");

    return SharedPtr(new VITPass(pScene, desc, dl));
}

void VITPass::renderScene(RenderContext* pContext, const Texture::SharedPtr disTex, const Falcor::Fbo::SharedPtr outputFbo, const VITPassSettings settings)
{
    Falcor::GraphicsVars::SharedPtr vars = Falcor::GraphicsVars::create(this->getProgram().get());
    vars->setTexture("disTex", disTex);

    if (settings.showTexRes) {
        vars->setTexture("voxTex", disTex);
    }

    vars["PerFrameCB"]["applyToModel"] = settings.applyToModel;
    vars["PerFrameCB2"]["showTexRes"] = settings.showTexRes;

    vars["PerFrameCB2"]["interp_min"] = settings.interp_min;
    vars["PerFrameCB2"]["interp_max"] = settings.interp_max;

    vars["PerFrameCB2"]["mipmapLevel"] = settings.mipmapLevel;

    //vars["PerFrameCB2"]["interp_min"] = float4(mpScene->getSceneBounds().minPoint, 1.f);
    //vars["PerFrameCB2"]["interp_max"] = float4(mpScene->getSceneBounds().maxPoint, 1.f);

    // This code sets the sampling from bi-linear to closest.
    /*
    Sampler::Desc desc;
    desc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point);

    Sampler::SharedPtr sampler = Sampler::create(desc);
    vars->setSampler("sampleWrap", sampler);
    */

    this->setVars(vars);

    mpState->setFbo(outputFbo);
    mpScene->rasterize(pContext, mpState.get(), mpVars.get(), RasterizerState::CullMode::None);
}
