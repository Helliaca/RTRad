#include "BaseRenderPass.h"

namespace RR {
    void BaseRenderPass::setScene(const Scene::SharedPtr& scene)
    {
        this->scene = scene;
    }

    void BaseRenderPass::render(RenderContext* pContext, const TextureGroup* tg)
    {
        setPerFrameVars(tg);
    }

    void BaseRenderPass::setPerFrameVars(const TextureGroup* textureGroup)
    {
    }

    BaseRenderPass::BaseRenderPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
    {
        scene = pScene;

        auto pProg = GraphicsProgram::create(progDesc, programDefines);

        state = GraphicsState::create();
        state->setProgram(pProg);

        vars = GraphicsVars::create(pProg.get());
    }

    BaseRenderPass::BaseRenderPass(const Scene::SharedPtr& pScene) : scene(pScene)
    {
    }

    void BaseRenderPass::UpdateVars()
    {
        vars = GraphicsVars::create(getProgram().get());
    }

    void BaseRenderPass::addProgramDefine(const std::string& name, const std::string& value, bool updateVars)
    {
        state->getProgram()->addDefine(name, value);
        if (updateVars) UpdateVars();
    }

    void BaseRenderPass::removeProgramDefine(const std::string& name, bool updateVars)
    {
        state->getProgram()->removeDefine(name);
        if (updateVars) UpdateVars();
    }

    void BaseRenderPass::setVars(const GraphicsVars::SharedPtr& pVars)
    {
        vars = pVars ? pVars : GraphicsVars::create(state->getProgram().get());
    }
}
