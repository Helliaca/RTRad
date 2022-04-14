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
    float texPerBatch;
    bool randomizeSample;
    bool useVisCache;
};

class RTLightmapPass : public RR::BaseRaytracePass, public RR::SettingsObject<RTLPassSettings>, public std::enable_shared_from_this<RTLightmapPass>
{
public:
    using SharedPtr = std::shared_ptr<RTLightmapPass>;

    static SharedPtr create(const Scene::SharedPtr& mpScene);

    void setPerFrameVars(const TextureGroup textureGroup) override;
    void render(RenderContext* pContext, const TextureGroup textureGroup) override;

    bool runBatch(RenderContext* pContext, const TextureGroup textureGroup);

    void onGuiRender(Gui* pGui, Gui::Window w);

private:
    RTLightmapPass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable);

    FullScreenPass::SharedPtr fsp;

    int batch_counter;

    int row_offset;
};

