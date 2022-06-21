#include "RTLightmapPass.h"

using namespace Falcor;

RTLightmapPass::SharedPtr RTLightmapPass::create(const Scene::SharedPtr& mpScene)
{
    // We'll now create a raytracing program. To do that we need to setup two things:
    // - A program description (RtProgram::Desc). This holds all shader entry points, compiler flags, macro defintions, etc.
    // - A binding table (RtBindingTable). This maps shaders to geometries in the scene, and sets the ray generation and miss shaders.
    //
    // After setting up these, we can create the RtProgram and associated RtProgramVars that holds the variable/resource bindings.
    // The RtProgram can be reused for different scenes, but RtProgramVars needs to binding table which is Scene-specific and
    // needs to be re-created when switching scene. In this example, we re-create both the program and vars when a scene is loaded.

    RtProgram::Desc rtProgDesc;

#if HEMISPHERIC_SAMPLING
    rtProgDesc.addShaderLibrary(RTLPASS_DIR_SHADERS"/rayTracedLightMap2.rt.hlsl");
#else
    rtProgDesc.addShaderLibrary(RTLPASS_DIR_SHADERS"/rayTracedLightMap.rt.hlsl");
#endif

    rtProgDesc.addDefines(mpScene->getSceneDefines());

    //rtProgDesc.addDefine("VISCACHE", "1");

    rtProgDesc.setMaxTraceRecursionDepth(1); // 1 for calling TraceRay from RayGen, 1 for calling it from the primary-ray ClosestHit shader for reflections, 1 for reflection ray tracing a shadow ray
    rtProgDesc.setMaxPayloadSize(24); // The largest ray payload struct (PrimaryRayData) is 24 bytes. The payload size should be set as small as possible for maximum performance.

    RtBindingTable::SharedPtr sbt = RtBindingTable::create(1, 1, mpScene->getGeometryCount());

    sbt->setRayGen(rtProgDesc.addRayGen("rayGen"));
    sbt->setMiss(0, rtProgDesc.addMiss("primaryMiss"));

#if HEMISPHERIC_SAMPLING
    // This code only if we run hit-shaders
    auto primary = rtProgDesc.addHitGroup("primaryClosestHit", "primaryAnyHit");
    sbt->setHitGroupByType(0, mpScene, Scene::GeometryType::TriangleMesh, primary);
#endif

    RTLightmapPass* pass = new RTLightmapPass(mpScene, rtProgDesc, sbt);

    return SharedPtr(pass);
}

static bool useVisCache = false;
void RTLightmapPass::setPerFrameVars(const TextureGroup* textureGroup)
{
    PROFILE("setPerFrameVars");

    // Viscache
    if (textureGroup->settings.useViscache != useVisCache) {
        useVisCache = textureGroup->settings.useViscache;
        addProgramDefine("VISCACHE", std::to_string(useVisCache), true);
    }
    if (useVisCache) {
        rtVars["vis"] = textureGroup->visBuf;
    }

    rtVars->setTexture("pos", textureGroup->posTex);
    rtVars->setTexture("nrm", textureGroup->nrmTex);
    rtVars->setTexture("arf", textureGroup->arfTex);
    rtVars->setTexture("mat", textureGroup->matTex);
    rtVars->setTexture("lig", textureGroup->lgiTex);
    rtVars->setTexture("lig2", textureGroup->lgoTex);
    rtVars->setTexture("voxTex", textureGroup->voxTex);

    rtVars["PerFrameCB"]["currentOffset"] = settings.currentOffset;

    rtVars["PerFrameCB"]["sampling_res"] = settings.sampling_res;
    rtVars["PerFrameCB"]["posOffset"] = scene->getSceneBounds().minPoint;
    rtVars["PerFrameCB"]["randomizeSamples"] = settings.underSamplingMethod == RTPassUndersampling::STATIC_RANDOMIZED;
    rtVars["PerFrameCB"]["texRes"] = textureGroup->lgiTex.get()->getWidth();
    rtVars["PerFrameCB"]["passNum"] = settings.passNum;
    rtVars["PerFrameCB"]["useSubstructuring"] = settings.underSamplingMethod == RTPassUndersampling::SUBSTRUCTURING;
}

void RTLightmapPass::render(RenderContext* pContext, const TextureGroup* textureGroup)
{
    PROFILE("renderRT");

    // Scene changes
    assert(scene);
    if (is_set(scene->getUpdates(), Scene::UpdateFlags::GeometryChanged))
    {
        throw std::runtime_error("This sample does not support scene geometry changes. Aborting.");
    }

    if (settings.currentOffset.x == 0 && settings.currentOffset.y == 0) {
        onBatchStarted(pContext, textureGroup);
    }

    // Get resolution
    glm::uint xres = textureGroup->lgoTex->getWidth(), yres = textureGroup->lgoTex->getHeight();

    uint2 batchVec = uint2(
        glm::clamp(settings.batchDims.x, glm::uint(1), xres - settings.currentOffset.x),
        glm::clamp(settings.batchDims.y, glm::uint(1), yres - settings.currentOffset.y)
    );
    

    // Set vars
    setPerFrameVars(textureGroup);    

    scene->raytrace(pContext, rtProgram.get(), rtVars, uint3(batchVec.x, batchVec.y, 1));

    // set row_offset
    settings.currentOffset.y += batchVec.y;

    // batch finished
    if (settings.currentOffset.x >= xres && settings.currentOffset.y >= yres) {
        onBatchComplete(pContext, textureGroup);
        settings.currentOffset = uint2(0, 0);
        settings.passNum++;
        settings.batchComplete = true;
    }
    // Row finished
    else if (settings.currentOffset.y >= yres) {
        settings.currentOffset.y = 0;
        settings.currentOffset.x += batchVec.x;
        settings.batchComplete = false;
    }
    else {
        settings.batchComplete = false;
    }
}

void RTLightmapPass::onRenderGui(Gui* Gui, Gui::Window* win)
{
    win->text("Undersampling Settings");

    Falcor::Gui::DropdownList usmlst;
    uint32_t usm = (uint32_t)settings.underSamplingMethod;
    usmlst.push_back({ (uint32_t)RTPassUndersampling::NONE, "None" });
    usmlst.push_back({ (uint32_t)RTPassUndersampling::STATIC_BILINEAR, "Static (Bilinear)" });
    usmlst.push_back({ (uint32_t)RTPassUndersampling::STATIC_RANDOMIZED, "Static (Randomized)" });
    usmlst.push_back({ (uint32_t)RTPassUndersampling::SUBSTRUCTURING, "Substructuring" });
    win->dropdown("Undersampling Method", usmlst, usm);
    settings.underSamplingMethod = (RTPassUndersampling)usm;

    if (settings.underSamplingMethod == RTPassUndersampling::STATIC_BILINEAR || settings.underSamplingMethod == RTPassUndersampling::STATIC_RANDOMIZED) {
        Falcor::Gui::DropdownList sreslst;
        uint32_t sres = settings.sampling_res;
        sreslst.push_back({ 1, "1x1" });
        sreslst.push_back({ 2, "2x2" });
        sreslst.push_back({ 4, "4x4" });
        sreslst.push_back({ 8, "8x8" });
        sreslst.push_back({ 16, "16x16" });
        win->dropdown("Sampling Res", sreslst, sres);
        settings.sampling_res = sres;
    }

    else if (settings.underSamplingMethod == RTPassUndersampling::SUBSTRUCTURING) {

        win->checkbox("Write Preview into LigIn", settings.writeSubstructurePreviewIntoLigIn);
        win->slider("Gradient Threshold", settings.subStructureSplitThreshold, 0.001f, 0.5f);
        Falcor::Gui::DropdownList ssreslst;
        uint32_t ssres = settings.subStructureNodeRes;
        ssreslst.push_back({ 2, "2x2" });
        ssreslst.push_back({ 4, "4x4" });
        ssreslst.push_back({ 8, "8x8" });
        ssreslst.push_back({ 16, "16x16" });
        win->dropdown("Max. Node Size", ssreslst, ssres);
        settings.subStructureNodeRes = ssres;
    }

    // What follows is a whole lot of math to ensure that only batching settings are allowed that lead to less than max_samples
    // of sample-steps are taken per batch.

    win->separator();
    win->text("Batching");

    int max_samples = MAX_SAMPLES_PER_BATCH;

    int samples_per_patch = (settings.textureResolution.x / settings.sampling_res) * (settings.textureResolution.y / settings.sampling_res);

    int max_batchsize = glm::min((int)(settings.textureResolution.x * settings.textureResolution.x), max_samples / samples_per_patch);

    int batch_width = glm::clamp(settings.batchDims.x, glm::uint(1), settings.textureResolution.x);
    win->slider("Batch width", batch_width, 1, (int)settings.textureResolution.x);

    glm::uint max_h = glm::min(max_batchsize / batch_width, (int)settings.textureResolution.y);
    int batch_height = glm::clamp(settings.batchDims.y, glm::uint(1), max_h);
    win->slider("Batch height", batch_height, 1, glm::min(max_batchsize/batch_width, (int)settings.textureResolution.y));

    int _batch_size = glm::clamp((int)(settings.batchDims.x * settings.batchDims.y), 1, max_batchsize);
    int batch_size = _batch_size;
    win->slider("Batch size", batch_size, 1, max_batchsize);

    int _sample_count = glm::clamp((int)(settings.batchDims.x*settings.batchDims.y * samples_per_patch), 1, max_samples);
    int sample_count = _sample_count;
    win->slider("Samples per batch", sample_count, 1 * samples_per_patch, max_batchsize*samples_per_patch);

    if (sample_count != _sample_count) {
        batch_size = sample_count / samples_per_patch;
    }

    if (batch_size != _batch_size) {
        batch_width = batch_height = (int)glm::sqrt(batch_size);
    }

    settings.batchDims.x = batch_width;
    settings.batchDims.y = batch_height;
}

void RTLightmapPass::onBatchStarted(RenderContext* pContext, const TextureGroup* textureGroup)
{
    if (settings.underSamplingMethod == RTPassUndersampling::SUBSTRUCTURING) {
        // Run refinement pass
        std::vector<Texture::SharedPtr> tfbo;
        tfbo.push_back(textureGroup->posTex);
        Fbo::SharedPtr fbo = Fbo::create(tfbo);
        fsp->getVars()->setTexture("lig", textureGroup->lgiTex);
        fsp->getVars()->setTexture("pos", textureGroup->posTex);
        fsp->getVars()->setTexture("nrm", textureGroup->nrmTex);
        fsp->getVars()["PerFrameCB"]["writeSubstructurePreviewIntoLigIn"] = settings.writeSubstructurePreviewIntoLigIn;
        fsp->getVars()["PerFrameCB"]["subStructureSplitThreshold"] = settings.subStructureSplitThreshold;

        int step = 1;
        while (step < settings.subStructureNodeRes)
        {
            fsp->getVars()["PerFrameCB"]["step"] = step;
            fsp->execute(pContext, fbo, true);
            step *= 2;
        }
    }
}

void RTLightmapPass::onBatchComplete(RenderContext* pContext, const TextureGroup* textureGroup)
{
}

RTLightmapPass::RTLightmapPass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable)
    : BaseRaytracePass(pScene, programDesc, bindingTable)
{
    settings = RTLPassSettings::RTLPassSettings();

    fsp = FullScreenPass::create(RTLPASS_DIR_SHADERS"/Ref.ps.hlsl", scene->getSceneDefines());
}
