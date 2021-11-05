#include "RTRad.h"
static const float4 kClearColor(0.38f, 0.52f, 0.10f, 1);
static const std::string kDefaultScene = DEFAULT_SCENE;

void RTRad::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "RTRad", { 300, 280 }, { 10, 80 });

    w.checkbox("Apply To Model", mApplyToModel);

    w.checkbox("Show Tex Res", showTexRes);

    w.slider("Sampling res", sampling_res, 1, 16);

    Falcor::Gui::DropdownList reslst;
    reslst.push_back({ 32, "32" });
    reslst.push_back({ 64, "64" });
    reslst.push_back({ 128, "128" });
    reslst.push_back({ 256, "256" });
    reslst.push_back({ 384, "384" });
    reslst.push_back({ 512, "512" });
    reslst.push_back({ 768, "768" });
    reslst.push_back({ 1024, "1024" });
    reslst.push_back({ 1536, "1536" });
    reslst.push_back({ 2048, "2048" });

    uint32_t prevRes = LigRes;
    w.dropdown("Lightmap Resolution", reslst, LigRes);

    w.slider("Tex per Batch", texPerBatch, 0.01f, 1.0f);

    mResetInputTextures = w.button("Reset Input Textures");

    makePass = w.button("Make Pass");

    if (LigRes != prevRes) {
        makeTextures();
        mResetInputTextures = true;
    }

    Falcor::Gui::DropdownList lst;
    lst.push_back({ 0, "posTex" });
    lst.push_back({ 1, "nrmTex" });
    lst.push_back({ 2, "arfTex" });
    lst.push_back({ 3, "matTex" });
    lst.push_back({ 4, "lgiTex" });
    lst.push_back({ 5, "lgoTex" });

    w.dropdown("Output Texture", lst, outputTex);

    w.text(output);


    Gui::Window w2(pGui, "Scene Settings", { 300, 400 }, { 10, 400 });

    if (w2.button("Load Scene"))
    {
        std::string filename;
        if (openFileDialog(Scene::getFileExtensionFilters(), filename))
        {
            loadScene(filename, gpFramework->getTargetFbo().get());
        }
    }

    mpScene->renderUI(w2);
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

    mResetInputTextures = true;
}

void RTRad::onLoad(RenderContext* pRenderContext)
{
    if (gpDevice->isFeatureSupported(Device::SupportedFeatures::Raytracing) == false)
    {
        logFatal("Device does not support raytracing!");
    }

    loadScene(kDefaultScene, gpFramework->getTargetFbo().get());
}

void RTRad::makeTextures()
{
    int res = LigRes;
    textureGroup.posTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    textureGroup.nrmTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    textureGroup.arfTex = Texture::create2D(res, res, Falcor::ResourceFormat::R32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    textureGroup.matTex = Texture::create2D(res, res, Falcor::ResourceFormat::R32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    textureGroup.lgiTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    textureGroup.lgoTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
}

void RTRad::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    if (mpScene)
    {
        {
            PROFILE("SceneUpdate");
            mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
        }

        {
            PROFILE("RTRad");

            if (makePass && !makeBatch) {
                std::swap(textureGroup.lgiTex, textureGroup.lgoTex);
                makeBatch = true;
                makePass = false;
                rttime = 0;
            }

            if (makeBatch) {
                makeBatch = !rtlPass->runBatch(pRenderContext, textureGroup, sampling_res, texPerBatch);
            }
        }

        {
            PROFILE("TextureClear");
            if (mResetInputTextures) {
                makeTextures();
                mpRasterPass->renderScene(pRenderContext, textureGroup);
                mResetInputTextures = false;
            }
        }

        {
            PROFILE("VITPass");
            Texture::SharedPtr t;
            switch (outputTex)
            {
            case 0: { t = textureGroup.posTex; break; }
            case 1: { t = textureGroup.nrmTex; break; }
            case 2: { t = textureGroup.arfTex; break; }
            case 3: { t = textureGroup.matTex; break; }
            case 4: { t = textureGroup.lgiTex; break; }
            case 5: { t = textureGroup.lgoTex; break; }
            default:
                t = textureGroup.posTex;
            }

            vitPass->renderScene(pRenderContext, t, pTargetFbo, mApplyToModel, t == textureGroup.matTex, showTexRes);
        }

        float gt = Profiler::instance().getEvent("/onFrameRender/RTRad")->getGpuTime();
        if (gt > 1.f) {
            rttime += gt;
        }
        output = "rad_time= "+std::to_string(rttime / 1000.f) + "s";
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
