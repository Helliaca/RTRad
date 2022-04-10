#include "VITPass.h"
#include <RTRad/Core/common.h>

using namespace Falcor;

VITPass::VITPass(const Scene::SharedPtr& pScene)
    : RR::BaseRasterPass(pScene, VITPASS_DIR_SHADERS"/VITP.vs.hlsl", VITPASS_DIR_SHADERS"/VITP.ps.hlsl")
{
    assert(pScene);
}

VITPass::SharedPtr VITPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a VITPass object without a scene");
    return SharedPtr(new VITPass(pScene));
}

void VITPass::render(RenderContext* pContext, const TextureGroup tg)
{
    BaseRasterPass::render(pContext, tg);

    // We render into the target FBO
    state->setFbo(tg.outputFbo);
    scene->rasterize(pContext, state.get(), vars.get(), RasterizerState::CullMode::None);
}

void VITPass::setPerFrameVars(const TextureGroup textureGroup)
{
    //TODO
    vars->setTexture("disTex", textureGroup.posTex);

    /*if (settings.showTexRes) {
        vars->setTexture("voxTex", disTex);
    }*/

    vars["PerFrameCB"]["applyToModel"] = settings.applyToModel;
    vars["PerFrameCB2"]["showTexRes"] = settings.showTexRes;

    vars["PerFrameCB2"]["interp_min"] = settings.interp_min;
    vars["PerFrameCB2"]["interp_max"] = settings.interp_max;

    vars["PerFrameCB2"]["mipmapLevel"] = settings.mipmapLevel;
}
