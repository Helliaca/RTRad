#include "CVMPass.h"
#include "../RTRad.h"

using namespace Falcor;

CVMPass::CVMPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
    : BaseGraphicsPass(progDesc, programDefines), mpScene(pScene)
{
    assert(pScene);
}

CVMPass::SharedPtr CVMPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a CVMPass object without a scene");

    Program::DefineList dl = Program::DefineList();
    dl.add(pScene->getSceneDefines());

    Falcor::Program::Desc desc;

    desc.addShaderLibrary(SHADERS_FOLDER"/CVMP.vs.hlsl");
    desc.vsEntry("main");

    desc.addShaderLibrary(SHADERS_FOLDER"/CVMP.gs.hlsl");
    desc.gsEntry("gmain");

    desc.addShaderLibrary(SHADERS_FOLDER"/CVMP.ps.hlsl");
    desc.psEntry("pmain");

    return SharedPtr(new CVMPass(pScene, desc, dl));
}

void CVMPass::renderScene(RenderContext* pContext, const TextureGroup tg)
{
    // Create FBO
    Texture::SharedPtr tmpTex = Texture::create2D(64, 64, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);

    std::vector<Texture::SharedPtr> tfbo;
    tfbo.push_back(tmpTex);
    Fbo::SharedPtr fbo = Fbo::create(tfbo);

    mpVars["PerFrameCB"]["posOffset"] = mpScene->getSceneBounds().minPoint;
    mpVars["voxTex"] = tg.voxTex;

    mpState->setFbo(fbo);
    mpScene->rasterize(pContext, mpState.get(), mpVars.get(), RasterizerState::CullMode::None);
}
