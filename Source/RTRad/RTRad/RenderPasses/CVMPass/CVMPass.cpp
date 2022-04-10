#include "CVMPass.h"

using namespace Falcor;

CVMPass::CVMPass(const Scene::SharedPtr& pScene)
    : RR::BaseRasterPass(pScene, CVMPASS_DIR_SHADERS"/CVMP.vs.hlsl", CVMPASS_DIR_SHADERS"/CVMP.gs.hlsl", CVMPASS_DIR_SHADERS"/CVMP.ps.hlsl")
{
    RasterizerState::Desc desc;
    desc.setConservativeRasterization(false);  // Maybe better as true?
    desc.setCullMode(RasterizerState::CullMode::None);
    desc.setDepthClamp(true);
    desc.setLineAntiAliasing(false);
    desc.setFrontCounterCW(true);
    rastState = RasterizerState::create(desc);
}

bool CVMPass::CurrentFboIsValid(const TextureGroup tg)
{
    if (voxFbo == nullptr) return false;
    return voxFbo->getHeight() == tg.voxTex->getHeight();
}

void CVMPass::MakeFbo(const TextureGroup tg)
{
    Texture::SharedPtr fboTex = Texture::create2D(tg.voxTex->getHeight(), tg.voxTex->getWidth(), Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    std::vector<Texture::SharedPtr> tfbo;
    tfbo.push_back(fboTex);
    voxFbo = Fbo::create(tfbo);
}

CVMPass::SharedPtr CVMPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a CVMPass object without a scene");
    return SharedPtr(new CVMPass(pScene));
}

void CVMPass::render(RenderContext* pContext, const TextureGroup tg)
{
    setPerFrameVars(tg);

    if (!CurrentFboIsValid(tg)) MakeFbo(tg);

    state->setFbo(voxFbo);
    scene->rasterize(pContext, state.get(), vars.get(), rastState, rastState);
}

void CVMPass::setPerFrameVars(const TextureGroup textureGroup)
{
    // Unused
    vars["PerFrameCB"]["posOffset"] = scene->getSceneBounds().minPoint;

    vars->setTexture("voxTex", textureGroup.voxTex);
}
