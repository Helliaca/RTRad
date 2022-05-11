#pragma once

#include "Falcor.h"
#include <RTRad/Core/BaseRaytracePass.h>
#include <RTRad/Core/common.h>
#include <RTRad/Core/SettingsObject.h>

using namespace Falcor;

enum class RTPassIntegral { AREA, HEMISPHERIC };

struct RTLPassSettings : public RR::BaseSettings {
    RTPassIntegral integral;
    int sampling_res;
    bool randomizeSample;

    bool batchComplete;
    int passNum;

    bool useSubstructuring;

    uint2 currentOffset;
    uint2 batchDims;

    // We need this for correct GUI output. It will be set each frame by RTRad
    uint2 textureResolution;

    RTLPassSettings() {
        integral = RTPassIntegral::AREA;
        sampling_res = 1;
        randomizeSample = false;

        batchComplete = true;
        passNum = 0;

        useSubstructuring = false;

        currentOffset = uint2(0, 0);
        batchDims = uint2(64, 1);

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

    FullScreenPass::SharedPtr fsp;
};

