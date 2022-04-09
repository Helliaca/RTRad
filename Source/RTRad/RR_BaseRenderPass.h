#pragma once

#include "RR_BasePipelineElement.h"

class RR_BaseRenderPass : public RR_BasePipelineElement
{
public:
    const Scene::SharedPtr& getScene() const { return mpScene; }

protected:
    Scene::SharedPtr mpScene;
};

