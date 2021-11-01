#pragma once

#include "Falcor.h"
#include "common.h"

using namespace Falcor;

class RTLightmapPass : public std::enable_shared_from_this<RTLightmapPass>
{
public:
    using SharedPtr = std::shared_ptr<RTLightmapPass>;

    static SharedPtr create();

    void load(const Scene::SharedPtr mpScene);
    void setPerFrameVars(const TextureGroup textureGroup, const int sampling_res);
    void renderRT(RenderContext* pContext, const TextureGroup textureGroup, const int sampling_res, const float texPerBatch);

    bool runBatch(RenderContext* pContext, const TextureGroup textureGroup, const int sampling_res, const float texPerBatch);

private:
    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    RtProgramVars::SharedPtr mpRtVars;

    FullScreenPass::SharedPtr fsp;

    Scene::SharedPtr mpScene;

    int batch_counter;

    int row_offset;
};

