#pragma once

#include "Falcor.h"
#include "../common.h"

class RTRad;

using namespace Falcor;


class RTRadGraphicsPass : protected BaseGraphicsPass
{
public:
    const Scene::SharedPtr& getScene() const { return mpScene; }
    

protected:
    RTRadGraphicsPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines);
    Scene::SharedPtr mpScene;
};

