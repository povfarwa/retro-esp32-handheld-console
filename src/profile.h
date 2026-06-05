#pragma once
#include "globals.h"

namespace Profile {
    extern bool needsRedraw;
    void init();
    void draw();
    void update();
    bool needsDraw();
    void setNeedsRedraw();
}
