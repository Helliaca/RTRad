#include "RTRad.h"
#include "Tools/SceneLoader.h"

void RTRad::onGuiRender(Gui* pGui)
{
    // MAIN CONTROLPANEL
    {
        Gui::Window w(pGui, "RTRad", { 300, 330 }, { 10, 80 });

        vitPass->onRenderGui(pGui, &w);

        rtlPass->onRenderGui(pGui, &w);

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

        uint32_t prevRes = mTextureRes;
        w.dropdown("Lightmap Resolution", reslst, mTextureRes);

        Falcor::Gui::DropdownList vreslst;
        vreslst.push_back({ 32, "32" });
        vreslst.push_back({ 64, "64" });
        vreslst.push_back({ 128, "128" });
        vreslst.push_back({ 256, "256" });

        uint32_t vprevRes = mVoxelRes;

        w.dropdown("VoxelMap Resolution", vreslst, mVoxelRes);

        mResetInputTextures = w.button("Reset Input Textures");

        mMakePass = w.button("Make Pass");

        if (mTextureRes != prevRes) {
            mResetInputTextures = true;
        }

        if (mVoxelRes != vprevRes) {
            mResetInputTextures = true;
        }

        if (rtlPass->settings.useVisCache && !textureGroup.visBuf) {
            mResetInputTextures = true;
        }

        w.text(mOutputString);
    }

    // SCENE CONTROLS
    {
        Gui::Window w2(pGui, "Scene Settings", { 300, 420 }, { 10, 400 });

        if (w2.button("Load Scene"))
        {
            std::string filename;
            if (openFileDialog(Scene::getFileExtensionFilters(), filename))
            {
                loadScene(filename);
            }
        }

        mpScene->renderUI(w2);
    }
}

void RTRad::loadScene(const std::string& filename)
{
    SceneLoader::LoadSceneFromFile(filename, mpScene, mpCamera);

    citPass = CITPass::create(mpScene);

    vitPass = VITPass::create(mpScene);

    rtlPass = RTLightmapPass::create(mpScene);

    cvmPass = CVMPass::create(mpScene);

    mResetInputTextures = true;
}

void RTRad::onLoad(RenderContext* pRenderContext)
{
    if (gpDevice->isFeatureSupported(Device::SupportedFeatures::Raytracing) == false)
    {
        logFatal("Device does not support raytracing!");
    }

    loadScene(DEFAULT_SCENE);
}

void RTRad::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), CLEAR_COLOR, 1.0f, 0, FboAttachmentType::All);

    if (mpScene)
    {
        {
            PROFILE("SceneUpdate");
            mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
        }

        {
            PROFILE("RTRad");

            if (mMakePass && !mMakeBatch) {
                std::swap(textureGroup.lgiTex, textureGroup.lgoTex);
                mMakeBatch = true;
                mMakePass = false;
                mAccTime = 0;
            }

            if (mMakeBatch) {
                rtlPass->render(pRenderContext, textureGroup);
                mMakeBatch = !rtlPass->settings.batchComplete;
                textureGroup.generateLMips(pRenderContext);
            }
        }

        {
            PROFILE("CITPass");
            if (mResetInputTextures) {
                // make new texgroup
                textureGroup = TextureGroup::makeTextures(mVoxelRes, mTextureRes, rtlPass->settings.useVisCache, pTargetFbo);

                // Create input data
                citPass->render(pRenderContext, textureGroup);

                // Create voxelmap (if used)
                cvmPass->render(pRenderContext, textureGroup);

                // Create mipmaps
                textureGroup.generateLMips(pRenderContext);

                mResetInputTextures = false;
            }
        }

        {
            PROFILE("VITPass");
            vitPass->render(pRenderContext, textureGroup);
        }

        float gt = Profiler::instance().getEvent("/onFrameRender/RTRad")->getGpuTime();
        if (gt > 1.f) {
            mAccTime += gt;
        }
        mOutputString = "rad_time= "+std::to_string(mAccTime / 1000.f) + "s";
    }

    TextRenderer::render(pRenderContext, gpFramework->getFrameRate().getMsg(), pTargetFbo, { 20, 20 });
}

void RTRad::onShutdown()
{
}

bool RTRad::onKeyEvent(const KeyboardEvent& keyEvent)
{
    /* //Old code to toggle raytracing with spacebar
    if (keyEvent.key == KeyboardEvent::Key::Space && keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        mRayTrace = !mRayTrace;
        return true;
    }
    */
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
}
