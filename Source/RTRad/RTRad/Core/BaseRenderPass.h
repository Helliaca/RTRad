#pragma once

#include "BasePipelineElement.h"
#include <RTRad/Core/TextureGroup.h>

namespace RR {
    class BaseRenderPass : public BasePipelineElement
    {
    public:
        virtual ~BaseRenderPass() = default;

        // Program
        void addProgramDefine(const std::string& name, const std::string& value = "", bool updateVars = false);
        void removeProgramDefine(const std::string& name, bool updateVars = false);
        GraphicsProgram::SharedPtr getProgram() const { return state->getProgram(); }

        // state
        const GraphicsState::SharedPtr& getState() const { return state; }

        // vars
        const GraphicsVars::SharedPtr& getVars() const { return vars; }
        ShaderVar getRootVar() const { return vars->getRootVar(); }
        void setVars(const GraphicsVars::SharedPtr& pVars);

        // scene
        const Scene::SharedPtr& getScene() const { return scene; }
        void setScene(const Scene::SharedPtr& scene);

        // render
        virtual void render(RenderContext* pContext, const TextureGroup* tg);
        virtual void setPerFrameVars(const TextureGroup* textureGroup);

    protected:
        BaseRenderPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
        BaseRenderPass(const Scene::SharedPtr& pScene);

        // Members
        Scene::SharedPtr scene;
        GraphicsVars::SharedPtr vars;
        GraphicsState::SharedPtr state;

    private:
        void UpdateVars();
    };
}

