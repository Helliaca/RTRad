#pragma once

#include "BaseRenderPass.h"

namespace RR {
    class BaseRaytracePass : protected BaseRenderPass
    {
    public:
        BaseRaytracePass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable);
    protected:
        RtProgram::SharedPtr rtProgram = nullptr;
        RtProgramVars::SharedPtr rtVars = nullptr;
    };
}

