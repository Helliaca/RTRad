#pragma once

#include "RR_BasePipelineElement.h"

class RR_BaseRenderPass : public RR_BasePipelineElement
{
public:
    virtual ~RR_BaseRenderPass() = default;

    void addDefine(const std::string& name, const std::string& value = "", bool updateVars = false);

    void removeDefine(const std::string& name, bool updateVars = false);

    GraphicsProgram::SharedPtr getProgram() const { return mpState->getProgram(); }

    const GraphicsState::SharedPtr& getState() const { return mpState; }

    const GraphicsVars::SharedPtr& getVars() const { return mpVars; }

    ShaderVar getRootVar() const { return mpVars->getRootVar(); }

    void setVars(const GraphicsVars::SharedPtr& pVars);

    const Scene::SharedPtr& getScene() const { return mpScene; }

protected:
    RR_BaseRenderPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);

    Scene::SharedPtr mpScene;

    GraphicsVars::SharedPtr mpVars;
    GraphicsState::SharedPtr mpState;
};

