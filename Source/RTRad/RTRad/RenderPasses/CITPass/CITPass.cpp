#include "CITPass.h"

using namespace Falcor;

CITPass::CITPass(const Scene::SharedPtr& pScene)
    : RR::BaseRasterPass(pScene, CITPASS_DIR_SHADERS"/CITP.vs.hlsl", CITPASS_DIR_SHADERS"/CITP.gs.hlsl", CITPASS_DIR_SHADERS"/CITP.ps.hlsl")
{
    setRastState(false);
}

void CITPass::setRastState(bool consv)
{
    RasterizerState::Desc desc;
    desc.setConservativeRasterization(consv);  // Maybe better as true?
    desc.setCullMode(RasterizerState::CullMode::None);
    //desc.setDepthClamp(true);
    //desc.setLineAntiAliasing(false);
    //desc.setFrontCounterCW(true);
    //desc.setScissorTest(false);
    //desc.setForcedSampleCount(1); // What does this do?
    rastState = RasterizerState::create(desc);
}

CITPass::SharedPtr CITPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a CITPass object without a scene");
    return SharedPtr(new CITPass(pScene));
}

void CITPass::render(RenderContext* pContext, const TextureGroup* tg)
{
    if (rastState->isConservativeRasterizationEnabled() != tg->settings.useConservativeRasterization) {
        setRastState(tg->settings.useConservativeRasterization);
    }

    setPerFrameVars(tg);

    // Create FBO. TODO: cache this?
    std::vector<Texture::SharedPtr> tfbo;
    tfbo.push_back(tg->posTex);
    tfbo.push_back(tg->nrmTex);
    tfbo.push_back(tg->arfTex);
    tfbo.push_back(tg->matTex);
    tfbo.push_back(tg->lgiTex);
    tfbo.push_back(tg->lgoTex);
    Fbo::SharedPtr fbo = Fbo::create(tfbo);

    state->setFbo(fbo);
    scene->rasterize(pContext, state.get(), vars.get(), rastState, rastState);
}

void CITPass::setPerFrameVars(const TextureGroup* textureGroup)
{
    vars["PerFrameCB"]["posOffset"] = scene->getSceneBounds().minPoint;
    vars["PerFrameCB_GS"]["texDims"] = glm::float2(textureGroup->lgoTex->getWidth(), textureGroup->lgoTex->getHeight());
}

void CITPass::onRenderGui(Gui* Gui, Gui::Window* win)
{
}
