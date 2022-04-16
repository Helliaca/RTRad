#pragma once

#include "Falcor.h"

using namespace Falcor;

namespace RR {
    class BasePipelineElement
    {
    public:
        virtual void onRenderGui(Gui* Gui, Gui::Window* win);
    };
}
