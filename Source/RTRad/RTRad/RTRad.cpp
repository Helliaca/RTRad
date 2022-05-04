#include "RTRad.h"
#include "Tools/SceneLoader.h"

RTRad::RTRad()
{
    textureGroup = new TextureGroup();
}

void RTRad::onGuiRender(Gui* pGui)
{
    // TEXGROUP
    {
        Gui::Window w(pGui, "TextureGroup Settings", { 300, 150 }, { 1600, 80 });
        textureGroup->onRenderGui(pGui, &w);
    }

    // VIS
    {
        Gui::Window w(pGui, "Visualization Settings", { 300, 150 }, { 1600, 240 });
        vitPass->onRenderGui(pGui, &w);
    }

    // MAIN CONTROLPANEL
    {
        Gui::Window w(pGui, "RTRad", { 300, 100 }, { 10, 80 });

        mResetInputTextures = w.button("Reset Input Textures");

        if (rtlPass->settings.batchComplete) {
            mMakePass = w.button("Make Pass");
        }

        if (Profiler::instance().isEnabled()) {
            w.text(mOutputString);
        }
    }

    // RTPASS
    {
        Gui::Window w(pGui, "Radiosity Settings", { 300, 200 }, { 10, 190 });

        rtlPass->onRenderGui(pGui, &w);
    }

    // SCENE CONTROLS
    {
        Gui::Window w(pGui, "Scene Settings", { 300, 420 }, { 10, 400 });

        if (w.button("Load Scene"))
        {
            std::string filename;
            if (openFileDialog(Scene::getFileExtensionFilters(), filename))
            {
                loadScene(filename);
            }
        }

        mpScene->renderUI(w);
    }

    // UPDATE VARS
    {
        mResetInputTextures = mResetInputTextures || textureGroup->settingsChanged;

        if (rtlPass->settings.useVisCache && !textureGroup->visBuf) {
            mResetInputTextures = true;
        }
    }
}

void RTRad::loadScene(const std::string& filename)
{
    SceneLoader::LoadSceneFromFile(filename, mpScene, mpCamera);

    citPass = CITPass::create(mpScene);

    vitPass = VITPass::create(mpScene);

    rtlPass = RTLightmapPass::create(mpScene);

    cvmPass = CVMPass::create(mpScene);

    // Reset tg settings
    textureGroup->settings = TextureGroupSettings();

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

            if (mMakePass) {
                std::swap(textureGroup->lgiTex, textureGroup->lgoTex);
                rtlPass->settings.batchComplete = false;
                mMakePass = false;
                mAccTime = 0;
            }

            if (!rtlPass->settings.batchComplete) {
                rtlPass->render(pRenderContext, textureGroup);
                textureGroup->generateLMips(pRenderContext);
            }
        }

        {
            PROFILE("CITPass");
            if (mResetInputTextures) {
                // make new texgroup
                textureGroup->RemakeTextures(pTargetFbo);

                // Create input data
                citPass->render(pRenderContext, textureGroup);

                // Create voxelmap (if used)
                cvmPass->render(pRenderContext, textureGroup);

                // Create mipmaps
                textureGroup->generateLMips(pRenderContext);

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
    delete textureGroup;
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
