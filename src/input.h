#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────
//  Input abstraction
//  Call Input::update() every frame.
//  Use pressed() / held() / axis() to read state.
//  Also populates g_input for game compatibility.
// ─────────────────────────────────────────────

namespace Input {

    enum Button {
        TOP    = 0,
        BOTTOM = 1,
        LEFT   = 2,
        RIGHT  = 3,
        SW     = 4,   // joystick click
        COUNT  = 5
    };

    struct Axis {
        int x;  // -100..+100, negative=left
        int y;  // -100..+100, negative=up
    };

    void  init();
    void  update();

    bool  pressed(Button b);
    bool  released(Button b);
    bool  held(Button b);
    Axis  axis();
    void  beep(int ms = 30);        // fire buzzer for ms
    void  beepTone(int ms, int repeatCount = 1);
}
