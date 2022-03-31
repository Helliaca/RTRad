#pragma once

#include "Falcor.h"
#include "../common.h"

using namespace Falcor;

struct RTLightmapPassSettings {
    int sampling_res;
    float texPerBatch;
    bool randomizeSample;

    RTLightmapPassSettings() {
        sampling_res = 1;
        texPerBatch = 1.0f;
        randomizeSample = false;
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
private:
    void load(const Scene::SharedPtr& mpScene);

    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    RtProgramVars::SharedPtr mpRtVars;

    FullScreenPass::SharedPtr fsp;

    Scene::SharedPtr mpScene;

    Buffer::SharedPtr visBuf;

    int batch_counter;

    int row_offset;
};

