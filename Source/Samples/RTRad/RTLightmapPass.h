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
    void setPerFrameVars(const TextureGroup textureGroup);
    void renderRT(RenderContext* pContext, const TextureGroup textureGroup);

    bool runBatch(RenderContext* pContext, const TextureGroup textureGroup);

private:
    RtProgram::SharedPtr mpRaytraceProgram = nullptr;
    RtProgramVars::SharedPtr mpRtVars;

    Scene::SharedPtr mpScene;

    int batch_counter;

    int row_offset;

    float texPerBatch = 0.25f;
};

