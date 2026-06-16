#pragma once
#include <Arduino.h>

namespace Input {

    enum Button {
        TOP    = 0,
        BOTTOM = 1,
        LEFT   = 2,
        RIGHT  = 3,
        SW     = 4,
        COUNT  = 5
    };

    struct Axis {
        int x;
        int y;
    };

    void  init();
    void  update();

    bool  pressed(Button b);
    bool  released(Button b);
    bool  held(Button b);
    Axis  axis();
    void  beep(int ms = 30);
    void  beepTone(int ms, int repeatCount = 1);
}
