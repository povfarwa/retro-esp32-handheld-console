#include "input.h"
#include "config.h"
#include "globals.h"
#include "sounds.h"

namespace Input {

static const uint8_t _pins[COUNT] = {
    PIN_BTN_TOP,
    PIN_BTN_BOTTOM,
    PIN_BTN_LEFT,
    PIN_BTN_RIGHT,
    PIN_JOY_SW
};

static bool _cur[COUNT]  = {};
static bool _prev[COUNT] = {};

static int _centerX = 2048;
static int _centerY = 2048;

void init() {
    for (int i = 0; i < COUNT; i++)
        pinMode(_pins[i], INPUT_PULLUP);

    pinMode(PIN_JOY_X, INPUT);
    pinMode(PIN_JOY_Y, INPUT);

    // Calibrate joystick centre
    delay(200);
    long sumX = 0, sumY = 0;
    for (int i = 0; i < 64; i++) {
        sumX += analogRead(PIN_JOY_X);
        sumY += analogRead(PIN_JOY_Y);
        delay(3);
    }
    _centerX = (int)(sumX / 64);
    _centerY = (int)(sumY / 64);
}

void update() {
    for (int i = 0; i < COUNT; i++) {
        _prev[i] = _cur[i];
        _cur[i]  = (digitalRead(_pins[i]) == LOW);
    }

    // Populate g_input for legacy game compatibility
    Axis a = axis();
    g_input.joyX    = a.x;
    g_input.joyY    = a.y;
    g_input.joyBtn  = _cur[SW];
    g_input.joyBtnP = pressed(SW);
    g_input.btnA    = _cur[RIGHT];
    g_input.btnAP   = pressed(RIGHT);
    g_input.btnB    = _cur[BOTTOM];
    g_input.btnBP   = pressed(BOTTOM);
    g_input.btnC    = _cur[TOP];
    g_input.btnCP   = pressed(TOP);
    g_input.btnD    = _cur[LEFT];
    g_input.btnDP   = pressed(LEFT);
}

bool pressed(Button b)  { return  _cur[b] && !_prev[b]; }
bool released(Button b) { return !_cur[b] &&  _prev[b]; }
bool held(Button b)     { return  _cur[b]; }

Axis axis() {
    int rawX = analogRead(PIN_JOY_X);
    int rawY = analogRead(PIN_JOY_Y);

    int dx = rawX - _centerX;
    int dy = rawY - _centerY;

    if (dx > -200 && dx < 200) dx = 0;
    if (dy > -200 && dy < 200) dy = 0;

    int nx = 0, ny = 0;
    if (dx != 0)
        nx = constrain(map(-dx, -2048, 2048, -100, 100), -100, 100);
    if (dy != 0)
        ny = constrain(map( dy, -2048, 2048, -100, 100), -100, 100);

    return {nx, ny};
}

void beep(int ms) {
    Sounds::tone(1000, ms);
}

void beepTone(int ms, int repeatCount) {
    for (int i = 0; i < repeatCount; i++) {
        Sounds::tone(1000, ms);
        if (i < repeatCount - 1) Sounds::silence(ms);
    }
}

} // namespace Input
