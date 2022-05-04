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
    bool useVisCache;

    bool batchComplete;
    int passNum;

    uint2 currentOffset;
    uint2 batchDims;

    RTLPassSettings() {
        integral = RTPassIntegral::AREA;
        sampling_res = 1;
        randomizeSample = false;
        useVisCache = false;

        batchComplete = true;
        passNum = 0;

        currentOffset = uint2(0, 0);
        batchDims = uint2(1, 1);
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

private:
    RTLightmapPass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable);

    FullScreenPass::SharedPtr fsp;
};

