#pragma once

#include "Falcor.h"
#include "../core/common.h"

using namespace Falcor;

enum class RTPassIntegral { AREA, HEMISPHERIC };

struct RTLightmapPassSettings {
    RTPassIntegral integral;
    int sampling_res;
    float texPerBatch;
    bool randomizeSample;
    bool useVisCache;

    RTLightmapPassSettings() {
        integral = RTPassIntegral::AREA;
        sampling_res = 1;
        texPerBatch = 1.0f;
        randomizeSample = false;
        useVisCache = false;
    }
};

class RTLightmapPass : public std::enable_shared_from_this<RTLightmapPass>
{
public:
    using SharedPtr = std::shared_ptr<RTLightmapPass>;

    static SharedPtr create(const Scene::SharedPtr& mpScene);

    void setPerFrameVars(const TextureGroup textureGroup, const RTLightmapPassSettings settings);
    void renderRT(RenderContext* pContext, const TextureGroup textureGroup, const RTLightmapPassSettings settings);

    bool runBatch(RenderContext* pContext, const TextureGroup textureGroup, const RTLightmapPassSettings settings);

    void onGuiRender(Gui* pGui, Gui::Window w);
private:
    void load(const Scene::SharedPtr& mpScene);

    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    RtProgramVars::SharedPtr mpRtVars;

    FullScreenPass::SharedPtr fsp;

    Scene::SharedPtr mpScene;

    int batch_counter;

    int row_offset;
};

