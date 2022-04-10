#include "BaseRasterPass.h"

namespace RR {
    BaseRasterPass::BaseRasterPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
        : BaseRenderPass(pScene, progDesc, programDefines)
    {
        rastState = RasterizerState::create(RasterizerState::Desc());
    }

    BaseRasterPass::BaseRasterPass(const Scene::SharedPtr& pScene, const std::string vertShader, const std::string fragShader)
        : BaseRasterPass(pScene, vertShader, "", fragShader)
    {
    }

    BaseRasterPass::BaseRasterPass(const Scene::SharedPtr& pScene, const std::string vertShader, const std::string geomShader, const std::string fragShader)
        : BaseRenderPass(pScene)
    {
        // Defines
        Program::DefineList dl = Program::DefineList();
        dl.add(pScene->getSceneDefines());

        // Program
        Falcor::Program::Desc desc;
        desc.addShaderLibrary(vertShader);
        desc.vsEntry("vmain");
        if (geomShader != "") {
            desc.addShaderLibrary(geomShader);
            desc.gsEntry("gmain");
        }
        desc.addShaderLibrary(fragShader);
        desc.psEntry("pmain");

        // create program
        auto pProg = GraphicsProgram::create(desc, dl);

        // create state
        state = GraphicsState::create();
        rastState = RasterizerState::create(RasterizerState::Desc());
        state->setProgram(pProg);

        // create vars
        vars = GraphicsVars::create(pProg.get());
    }
}
