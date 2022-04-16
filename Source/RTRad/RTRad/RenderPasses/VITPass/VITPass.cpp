#include "VITPass.h"
#include <RTRad/Core/common.h>
#include "Tools/SceneLoader.h"

using namespace Falcor;

VITPass::VITPass(const Scene::SharedPtr& pScene)
    : RR::BaseRasterPass(pScene, VITPASS_DIR_SHADERS"/VITP.vs.hlsl", VITPASS_DIR_SHADERS"/VITP.ps.hlsl")
{
    assert(pScene);
    Camera::SharedPtr cam = scene->getCamera();
    SceneLoader::LoadSceneFromFile(VITPASS_DIR_UVPLANESCENE, UVPlaneScene, cam);
}

VITPass::SharedPtr VITPass::create(const Scene::SharedPtr& pScene)
{
    if (pScene == nullptr) throw std::exception("Can't create a VITPass object without a scene");
    return SharedPtr(new VITPass(pScene));
}

void VITPass::render(RenderContext* pContext, const TextureGroup* tg)
{
    setPerFrameVars(tg);

    // We render into the target FBO
    state->setFbo(tg->outputFbo);
    if (settings.maskGeometry) {
        scene->rasterize(pContext, state.get(), vars.get(), RasterizerState::CullMode::None);
    }
    else
    {
        UVPlaneScene->rasterize(pContext, state.get(), vars.get(), RasterizerState::CullMode::None);
    }
}

void VITPass::setPerFrameVars(const TextureGroup* textureGroup)
{
    // Set output texture and interpolation-values
    Texture::SharedPtr t;
    settings.interp_min = float4(0.0f);
    settings.interp_max = float4(1.0f);

    settings.showVoxelMap = false;
    switch (settings.outputTexture)
    {
        case 0: {
            t = textureGroup->posTex;
            settings.interp_min = float4(scene->getSceneBounds().minPoint, 1.f);
            settings.interp_max = float4(scene->getSceneBounds().maxPoint, 1.f);
            break;
        }
        case 1: { t = textureGroup->nrmTex; break; }
        case 2: { t = textureGroup->arfTex; break; }
        case 3: { t = textureGroup->matTex; break; }
        case 4: { t = textureGroup->lgiTex; break; }
        case 5: { t = textureGroup->lgoTex; break; }
        case 6: { t = nullptr; settings.showVoxelMap = true; break; }
        default: { t = textureGroup->posTex; break; }
    }

    if (!settings.showVoxelMap || textureGroup->voxTex == nullptr) {
        vars->setTexture("outputTex", t);
    }
    else
    {
        vars->setTexture("voxTex", textureGroup->voxTex);
    }

    vars["PerFrameCB_vs"]["applyToModel"] = settings.applyToModel;

    vars["PerFrameCB_ps"]["showTexRes"] = settings.showTexRes;
    vars["PerFrameCB_ps"]["showVoxelMap"] = settings.showVoxelMap;
    vars["PerFrameCB_ps"]["interp_min"] = settings.interp_min;
    vars["PerFrameCB_ps"]["interp_max"] = settings.interp_max;
    vars["PerFrameCB_ps"]["mipmapLevel"] = settings.mipmapLevel;

    vars["PerFrameCB_ps"]["minPos"] = scene->getSceneBounds().minPoint;
    vars["PerFrameCB_ps"]["maxPos"] = scene->getSceneBounds().maxPoint;
}

void VITPass::onRenderGui(Gui* Gui, Gui::Window* win)
{
    win->text("VITPass Settings");

    Falcor::Gui::DropdownList lst;
    lst.push_back({ 0, "posTex" });
    lst.push_back({ 1, "nrmTex" });
    lst.push_back({ 2, "arfTex" });
    lst.push_back({ 3, "matTex" });
    lst.push_back({ 4, "lgiTex" });
    lst.push_back({ 5, "lgoTex" });
    lst.push_back({ 6, "voxTex" });

    win->dropdown("Output Texture", lst, settings.outputTexture);

    win->checkbox("Apply To Model", settings.applyToModel);

    if (!settings.applyToModel) {
        win->checkbox("Mask Geometry", settings.maskGeometry);
    }
    else
    {
        settings.maskGeometry = true;
    }

    win->checkbox("Show Tex Res", settings.showTexRes);

    win->slider("Mipmaplevel", settings.mipmapLevel, 0, DEFAULT_MIPMAP_LEVELS);
}
