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
        Gui::Window w(pGui, "TextureGroup Settings", { 350, 150 }, { 1080, 80 });
        textureGroup->onRenderGui(pGui, &w);
    }

    // VIS
    {
        Gui::Window w(pGui, "Visualization Settings", { 350, 300 }, { 1080, 240 });
        vitPass->onRenderGui(pGui, &w);
    }

    // MAIN CONTROLPANEL
    {
        Gui::Window w(pGui, "RTRad", { 350, 100 }, { 10, 80 });

        mResetInputTextures = w.button("Reset");

        if (rtlPass->settings.batchComplete) {
            mMakePass = w.button("Make Pass");
        }

        if (Profiler::instance().isEnabled()) {
            w.text(mOutputString);
        }

        float fov = mpCamera->getFocalLength();
        w.slider("Camera Focal Length", fov, 1.0f, 100.0f, false);
        mpCamera->setFocalLength(fov);
    }

    // RTPASS
    {
        Gui::Window w(pGui, "Radiosity Settings", { 350, 400 }, { 10, 190 });

        rtlPass->settings.textureResolution = uint2(textureGroup->settings.textureResolution, textureGroup->settings.textureResolution);

        rtlPass->onRenderGui(pGui, &w);
    }

    // SCENE CONTROLS
    {
        Gui::Window w(pGui, "Scene Settings", { 350, 220 }, { 10, 600 });

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

                rtlPass->settings.batchComplete = true;
                rtlPass->settings.passNum = 0;

                mResetInputTextures = false;
            }
        }

        {
            PROFILE("VITPass");
            vitPass->render(pRenderContext, textureGroup);
        }

        float gt = Profiler::instance().getEvent("/onFrameRender/RTRad")->getGpuTime();
        if (!rtlPass->settings.batchComplete || gt > 1.0f) {
            mAccTime += gt;
        }
        mOutputString = "rad_time= "+std::to_string(mAccTime / 1000.f) + "s";
    }

    // Disable this line to disable the FPB counter. Then you can press F2 to hide the GUI and take proper screenshots.
    //TextRenderer::render(pRenderContext, gpFramework->getFrameRate().getMsg(), pTargetFbo, { 20, 20 });
}

void RTRad::onShutdown()
{
    delete textureGroup;
}

bool RTRad::onKeyEvent(const KeyboardEvent& keyEvent)
{
    if (keyEvent.key == KeyboardEvent::Key::Escape && keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
        rtlPass->settings.batchComplete = true;
        mMakePass = false;
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
    if (mpCamera)
    {
        float aspectRatio = ((float)width / (float)height);
        mpCamera->setAspectRatio(aspectRatio);
    }
}
