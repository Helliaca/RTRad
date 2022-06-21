#include "BaseRaytracePass.h"

RR::BaseRaytracePass::BaseRaytracePass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable) : BaseRenderPass(pScene)
{
    rtBindingTable = bindingTable;

    // Create RtProgram from RtProgram::Desc
    rtProgram = RtProgram::create(programDesc);
    //state->setProgram(std::static_pointer_cast<GraphicsProgram>(rtP));    

    // Create RtVars from ShaderBindingTable
    rtVars = RtProgramVars::create(rtProgram, bindingTable);
    //vars = std::static_pointer_cast<GraphicsVars>(rtVars);
}

void RR::BaseRaytracePass::addProgramDefine(const std::string& name, const std::string& value, bool updateVars)
{
    rtProgram->addDefine(name, value);
    if (updateVars) {
        rtVars = RtProgramVars::create(rtProgram, rtBindingTable);
    }
}

void RR::BaseRaytracePass::removeProgramDefine(const std::string& name, bool updateVars)
{
    rtProgram->removeDefine(name);
    if (updateVars) {
        rtVars = RtProgramVars::create(rtProgram, rtBindingTable);
    }
}
