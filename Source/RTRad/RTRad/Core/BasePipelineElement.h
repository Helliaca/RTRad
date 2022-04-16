#pragma once

#include "Falcor.h"

using namespace Falcor;

namespace RR {
    struct BasePipelineElement
    {
    public:
        virtual void onRenderGui(Gui* Gui, Gui::Window* win) = 0;
    };
}
