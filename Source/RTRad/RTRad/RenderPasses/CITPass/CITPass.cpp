#include "CITPass.h"

using namespace Falcor;

CITPass::CITPass(const Scene::SharedPtr& pScene)
    : RR::BaseRasterPass(pScene, CITPASS_DIR_SHADERS"/CITP.vs.hlsl", CITPASS_DIR_SHADERS"/CITP.gs.hlsl", CITPASS_DIR_SHADERS"/CITP.ps.hlsl")
{
    
}

CITPass::SharedPtr CITPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a CITPass object without a scene");
    return SharedPtr(new CITPass(pScene));
}

void CITPass::render(RenderContext* pContext, const TextureGroup tg)
{
    setPerFrameVars(tg);

    // Create FBO. TODO: cache this?
    std::vector<Texture::SharedPtr> tfbo;
    tfbo.push_back(tg.posTex);
    tfbo.push_back(tg.nrmTex);
    tfbo.push_back(tg.arfTex);
    tfbo.push_back(tg.matTex);
    tfbo.push_back(tg.lgiTex);
    tfbo.push_back(tg.lgoTex);
    Fbo::SharedPtr fbo = Fbo::create(tfbo);

    state->setFbo(fbo);
    scene->rasterize(pContext, state.get(), vars.get(), RasterizerState::CullMode::None);
}

void CITPass::setPerFrameVars(const TextureGroup textureGroup)
{
    vars["PerFrameCB"]["posOffset"] = scene->getSceneBounds().minPoint;
}
