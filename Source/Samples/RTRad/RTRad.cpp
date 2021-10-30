#include "RTRad.h"
static const float4 kClearColor(0.38f, 0.52f, 0.10f, 1);
static const std::string kDefaultScene = "RTRad/mrad.pyscene";

void RTRad::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Hello DXR Settings", { 300, 400 }, { 10, 80 });

    w.checkbox("Ray Trace", mRayTrace);
    w.checkbox("Use Depth of Field", mUseDOF);

    w.checkbox("Apply To Model", mApplyToModel);

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

    // We'll now create a raytracing program. To do that we need to setup two things:
    // - A program description (RtProgram::Desc). This holds all shader entry points, compiler flags, macro defintions, etc.
    // - A binding table (RtBindingTable). This maps shaders to geometries in the scene, and sets the ray generation and miss shaders.
    //
    // After setting up these, we can create the RtProgram and associated RtProgramVars that holds the variable/resource bindings.
    // The RtProgram can be reused for different scenes, but RtProgramVars needs to binding table which is Scene-specific and
    // needs to be re-created when switching scene. In this example, we re-create both the program and vars when a scene is loaded.

    RtProgram::Desc rtProgDesc;
    rtProgDesc.addShaderLibrary("Samples/HelloDXR/HelloDXR.rt.slang");
    rtProgDesc.addDefines(mpScene->getSceneDefines());
    rtProgDesc.setMaxTraceRecursionDepth(3); // 1 for calling TraceRay from RayGen, 1 for calling it from the primary-ray ClosestHit shader for reflections, 1 for reflection ray tracing a shadow ray
    rtProgDesc.setMaxPayloadSize(24); // The largest ray payload struct (PrimaryRayData) is 24 bytes. The payload size should be set as small as possible for maximum performance.

    RtBindingTable::SharedPtr sbt = RtBindingTable::create(2, 2, mpScene->getGeometryCount());
    sbt->setRayGen(rtProgDesc.addRayGen("rayGen"));
    sbt->setMiss(0, rtProgDesc.addMiss("primaryMiss"));
    sbt->setMiss(1, rtProgDesc.addMiss("shadowMiss"));
    auto primary = rtProgDesc.addHitGroup("primaryClosestHit", "primaryAnyHit");
    auto shadow = rtProgDesc.addHitGroup("", "shadowAnyHit");
    sbt->setHitGroupByType(0, mpScene, Scene::GeometryType::TriangleMesh, primary);
    sbt->setHitGroupByType(1, mpScene, Scene::GeometryType::TriangleMesh, shadow);

    mpRaytraceProgram = RtProgram::create(rtProgDesc);
    mpRtVars = RtProgramVars::create(mpRaytraceProgram, sbt);
}

void RTRad::onLoad(RenderContext* pRenderContext)
{
    if (gpDevice->isFeatureSupported(Device::SupportedFeatures::Raytracing) == false)
    {
        logFatal("Device does not support raytracing!");
    }

    loadScene(kDefaultScene, gpFramework->getTargetFbo().get());

    int res = 128;
    posTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    nrmTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    li0Tex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    li1Tex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, 1, (const void*)nullptr, Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
}

void RTRad::setPerFrameVars(const Fbo* pTargetFbo)
{
    PROFILE("setPerFrameVars");
    auto cb = mpRtVars["PerFrameCB"];
    cb["invView"] = glm::inverse(mpCamera->getViewMatrix());
    cb["viewportDims"] = float2(pTargetFbo->getWidth(), pTargetFbo->getHeight());
    float fovY = focalLengthToFovY(mpCamera->getFocalLength(), Camera::kDefaultFrameHeight);
    cb["tanHalfFovY"] = std::tan(fovY * 0.5f);
    cb["sampleIndex"] = mSampleIndex++;
    cb["useDOF"] = mUseDOF;
    mpRtVars->getRayGenVars()["gOutput"] = mpRtOut;
}

void RTRad::renderRT(RenderContext* pContext, const Fbo* pTargetFbo)
{
    PROFILE("renderRT");

    assert(mpScene);
    if (is_set(mpScene->getUpdates(), Scene::UpdateFlags::GeometryChanged))
    {
        throw std::runtime_error("This sample does not support scene geometry changes. Aborting.");
    }

    setPerFrameVars(pTargetFbo);

    pContext->clearUAV(mpRtOut->getUAV().get(), kClearColor);
    mpScene->raytrace(pContext, mpRaytraceProgram.get(), mpRtVars, uint3(pTargetFbo->getWidth(), pTargetFbo->getHeight(), 1));
    pContext->blit(mpRtOut->getSRV(), pTargetFbo->getRenderTargetView(0));
}

void RTRad::onFrameRender(RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    pRenderContext->clearFbo(pTargetFbo.get(), kClearColor, 1.0f, 0, FboAttachmentType::All);

    if (mpScene)
    {
        mpScene->update(pRenderContext, gpFramework->getGlobalClock().getTime());
        if (mRayTrace) {
            //renderRT(pRenderContext, pTargetFbo.get());
            rtlPass->renderRT(pRenderContext, pTargetFbo.get(), mpCamera);
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
            mpRasterPass->renderScene(pRenderContext, posTex, nrmTex, li0Tex, li1Tex);

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
