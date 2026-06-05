#pragma once
#include "globals.h"

namespace Settings {
    extern bool needsRedraw;
    void init();
    void draw();
    void update();
    bool needsDraw();
    void setNeedsRedraw();
}
