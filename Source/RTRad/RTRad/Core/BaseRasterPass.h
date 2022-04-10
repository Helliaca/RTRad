#pragma once

#include "BaseRenderPass.h"

namespace RR {
    class BaseRasterPass : protected BaseRenderPass
    {
    public:
        const RasterizerState::SharedPtr& getRasterizerState() const {
            return rastState;
        }

    protected:
        RasterizerState::SharedPtr rastState;

        BaseRasterPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
        BaseRasterPass(const Scene::SharedPtr& pScene, const std::string vertShader, const std::string geomShader, const std::string fragShader);
        BaseRasterPass(const Scene::SharedPtr& pScene, const std::string vertShader, const std::string fragShader);
    };
}

