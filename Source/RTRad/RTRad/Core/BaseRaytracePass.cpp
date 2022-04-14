#include "BaseRaytracePass.h"

RR::BaseRaytracePass::BaseRaytracePass(const Scene::SharedPtr& pScene, const RtProgram::Desc programDesc, const RtBindingTable::SharedPtr bindingTable) : BaseRenderPass(pScene)
{
    // Create RtProgram from RtProgram::Desc
    rtProgram = RtProgram::create(programDesc);
    //state->setProgram(std::static_pointer_cast<GraphicsProgram>(rtP));    

    // Create RtVars from ShaderBindingTable
    rtVars = RtProgramVars::create(rtProgram, bindingTable);
    //vars = std::static_pointer_cast<GraphicsVars>(rtVars);
}
