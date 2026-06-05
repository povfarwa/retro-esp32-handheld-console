#pragma once
#include "globals.h"

namespace Menu {
    extern bool needsRedraw;
    void init();
    void draw();
    void update();   // handles navigation + selection
    bool needsDraw();
    void setNeedsRedraw();
}
