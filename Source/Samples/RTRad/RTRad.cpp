#include "RTRad.h"
static const float4 kClearColor(0.38f, 0.52f, 0.10f, 1);
static const std::string kDefaultScene = "RTRad/mrad.pyscene";

void RTRad::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Hello DXR Settings", { 300, 400 }, { 10, 80 });

    w.checkbox("Ray Trace", mRayTrace);
    w.checkbox("Use Depth of Field", mUseDOF);

    w.checkbox("Apply To Model", mApplyToModel);

    w.checkbox("Reset Input TExtures", mResetInputTextures);

    Falcor::Gui::DropdownList lst;
    lst.push_back({ 0, "posTex" });
    lst.push_back({ 1, "nrmTex" });
    lst.push_back({ 2, "li0Tex" });
    lst.push_back({ 3, "li1Tex" });

    w.dropdown("Output Texture", lst, outputTex);

    if (w.button("Load Scene"))
    {
        std::string filename;
        if (openFileDialog(Scene::getFileExtensionFilters(), filename))
        {
            loadScene(filename, gpFramework->getTargetFbo().get());
        }
    }

    mpScene->renderUI(w);
}

void RTRad::loadScene(const std::string& filename, const Fbo* pTargetFbo)
{
    mpScene = Scene::create(filename);
    if (!mpScene) return;

    mpCamera = mpScene->getCamera();

    // Update the controllers
    float radius = mpScene->getSceneBounds().radius();
    mpScene->setCameraSpeed(radius * 0.25f);
    float nearZ = std::max(0.1f, radius / 750.0f);
    float farZ = radius * 10;
    mpCamera->setDepthRange(nearZ, farZ);
    mpCamera->setAspectRatio((float)pTargetFbo->getWidth() / (float)pTargetFbo->getHeight());

    mpRasterPass = CITPass::create(mpScene);

    vitPass = VITPass::create(mpScene);

    rtlPass = RTLightmapPass::create();
    rtlPass->load(mpScene);
}

void RTRad::onLoad(RenderContext* pRenderContext)
{
    if (gpDevice->isFeatureSupported(Device::SupportedFeatures::Raytracing) == false)
    {
        logFatal("Device does not support raytracing!");
    }

    loadScene(kDefaultScene, gpFramework->getTargetFbo().get());

    int res = 128;
    posTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    nrmTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    li0Tex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    li1Tex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
}

void RTRad::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    if (mpScene)
    {
        mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
        if (mRayTrace) {
            //renderRT(pRenderContext, pTargetFbo.get());
            rtlPass->renderRT(pRenderContext, pTargetFbo.get(), mpCamera, posTex, nrmTex, li0Tex, li1Tex);
        }
        else {
            //Falcor::GraphicsVars rasterVars = Falcor::GraphicsVars::create()
            //mpRasterPass->getVars()->setVariable("posTex", posTex);
            //mpRasterPass->setVars()
            //mpRasterPass->getVars()["posTex"] = posTex;

            //Falcor::GraphicsVars::SharedPtr vars = Falcor::GraphicsVars::create(mpRasterPass->getProgram().get());
            //vars->setTexture("posTex", posTex);
            //mpRasterPass->setVars(vars);

            //create fbo
            //std::vector<Texture::SharedPtr> tfbo;
            //tfbo.push_back(posTex);

            //Fbo::SharedPtr fbo = Fbo::create(tfbo);

            //render to fbo
            //mpRasterPass->renderScene(pRenderContext, fbo);
            if (mResetInputTextures) {
                mpRasterPass->renderScene(pRenderContext, posTex, nrmTex, li0Tex, li1Tex);
            }

            Texture::SharedPtr t;
            switch (outputTex)
            {
            case 0: { t = posTex; break; }
            case 1: { t = nrmTex; break; }
            case 2: { t = li0Tex; break; }
            case 3: { t = li1Tex; break; }
            default:
                t = posTex;
            }

            vitPass->renderScene(pRenderContext, t, pTargetFbo, mApplyToModel);

            //copy to output render-view
            //pRenderContext->blit(posTex->getSRV(), pTargetFbo->getRenderTargetView(0));

            //mpRasterPass->renderScene(pRenderContext, pTargetFbo);
            //mpRasterPass->renderScene(pRenderContext, mpRtOut-)
        }
    }

    TextRenderer::render(pRenderContext, gpFramework->getFrameRate().getMsg(), pTargetFbo, { 20, 20 });
}

void RTRad::onShutdown()
{
}

bool RTRad::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.key == KeyboardEvent::Key::Space && keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        mRayTrace = !mRayTrace;
        return true;
    }
    if (mpScene && mpScene->onKeyEvent(keyEvent)) return true;
    return false;
}

bool RTRad::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpScene && mpScene->onMouseEvent(mouseEvent);
}

void RTRad::onHotReload(HotReloadFlags reloaded)
{
}

void RTRad::onResizeSwapChain(uint32_t width, uint32_t height)
{
    float h = (float)height;
    float w = (float)width;

    if (mpCamera)
    {
        mpCamera->setFocalLength(18);
        float aspectRatio = (w / h);
        mpCamera->setAspectRatio(aspectRatio);
    }

    mpRtOut = Texture::create2D(width, height, ResourceFormat::RGBA16Float, 1, 1, nullptr, Resource::BindFlags::UnorderedAccess | Resource::BindFlags::ShaderResource);
}
