#pragma once

#include "Falcor.h"
#include "common.h"

namespace RR {
    class BasePipelineElement
    {
    public:
        virtual void onRenderGui(Gui* Gui, Gui::Window* win);
    };
}
