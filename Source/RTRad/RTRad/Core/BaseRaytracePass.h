#pragma once

#include "BaseRenderPass.h"

namespace RR {
    class BaseRaytracePass : protected BaseRenderPass
    {
    public:
        BaseRaytracePass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable);

        void addProgramDefine(const std::string& name, const std::string& value = "", bool updateVars = false) override;
        void removeProgramDefine(const std::string& name, bool updateVars = false) override;
    protected:
        RtProgram::SharedPtr rtProgram = nullptr;
        RtProgramVars::SharedPtr rtVars = nullptr;
        RtBindingTable::SharedPtr rtBindingTable = nullptr;
    };
}

