#include "BaseRenderPass.h"

namespace RR {
    BaseRenderPass::BaseRenderPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
    {
        mpScene = pScene;

        auto pProg = GraphicsProgram::create(progDesc, programDefines);

        mpState = GraphicsState::create();
        mpState->setProgram(pProg);

        mpVars = GraphicsVars::create(pProg.get());
    }

    void BaseRenderPass::addDefine(const std::string& name, const std::string& value, bool updateVars)
    {
        mpState->getProgram()->addDefine(name, value);
        if (updateVars) mpVars = GraphicsVars::create(mpState->getProgram().get());
    }

    void BaseRenderPass::removeDefine(const std::string& name, bool updateVars)
    {
        mpState->getProgram()->removeDefine(name);
        if (updateVars) mpVars = GraphicsVars::create(mpState->getProgram().get());
    }

    void BaseRenderPass::setVars(const GraphicsVars::SharedPtr& pVars)
    {
        mpVars = pVars ? pVars : GraphicsVars::create(mpState->getProgram().get());
    }
}
