#include "RTRadGraphicsPass.h"

RTRadGraphicsPass::RTRadGraphicsPass(const Scene::SharedPtr& pScene, const Program::Desc& progDesc, const Program::DefineList& programDefines)
    :
    BaseGraphicsPass(progDesc, programDefines),
    mpScene(pScene)
{
    assert(pScene);
}
