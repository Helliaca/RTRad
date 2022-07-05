#pragma once

#include "Falcor.h"
#include <RTRad/Core/BaseRaytracePass.h>
#include <RTRad/Core/common.h>
#include <RTRad/Core/SettingsObject.h>

using namespace Falcor;

enum class RTPassIntegral { AREA=0, HEMISPHERIC=1 };
enum class RTPassUndersampling { NONE=0, STATIC_BILINEAR=1, STATIC_RANDOMIZED=2, SUBSTRUCTURING=3 };

struct RTLPassSettings : public RR::BaseSettings {
    RTPassIntegral integral;

    bool batchComplete;
    int passNum;

    RTPassUndersampling underSamplingMethod;

    int sampling_res;
    int hemisphere_samples;

    bool writeSubstructurePreviewIntoLigIn;
    float subStructureSplitThreshold;
    int subStructureNodeRes;

    bool useVoxelRaymarching;
    int voxelRaymarchRatio;

    uint2 currentOffset;
    uint2 batchDims;

    float reflectivity_factor;
    float distance_factor;

    bool runExtendSeamsPass;

    // We need this for correct GUI output. It will be set each frame by RTRad
    uint2 textureResolution;

    RTLPassSettings() {
        integral = RTPassIntegral::AREA;
        sampling_res = 1;
        hemisphere_samples = 100;

        batchComplete = true;
        passNum = 0;

        underSamplingMethod = RTPassUndersampling::NONE;

        writeSubstructurePreviewIntoLigIn = false;
        subStructureSplitThreshold = 0.05f;
        subStructureNodeRes = 4;

        useVoxelRaymarching = false;
        voxelRaymarchRatio = 4;

        currentOffset = uint2(0, 0);
        batchDims = uint2(64, 1);

        reflectivity_factor = 0.9f;
        distance_factor = 1.0f;

        runExtendSeamsPass = true;

        textureResolution = uint2(64, 64);
    }
};

class RTLightmapPass : public RR::BaseRaytracePass, public RR::SettingsObject<RTLPassSettings>, public std::enable_shared_from_this<RTLightmapPass>
{
public:
    using SharedPtr = std::shared_ptr<RTLightmapPass>;

    static SharedPtr create(const Scene::SharedPtr& mpScene);

    void setPerFrameVars(const TextureGroup* textureGroup) override;
    void render(RenderContext* pContext, const TextureGroup* textureGroup) override;

    void onRenderGui(Gui* Gui, Gui::Window* win) override;

    void onBatchStarted(RenderContext* pContext, const TextureGroup* textureGroup);
    void onBatchComplete(RenderContext* pContext, const TextureGroup* textureGroup);

private:
    RTLightmapPass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable);

    FullScreenPass::SharedPtr SubstructurePass;
    FullScreenPass::SharedPtr SeamExtendPass;
};

