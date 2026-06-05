#include "input.h"

// Previous raw button states for edge detection
static bool _prevJoy, _prevA, _prevB, _prevC, _prevD;

void Input::init() {
    pinMode(PIN_JOY_SW, INPUT_PULLUP);
    pinMode(PIN_BTN_A,  INPUT_PULLUP);
    pinMode(PIN_BTN_B,  INPUT_PULLUP);
    pinMode(PIN_BTN_C,  INPUT_PULLUP);
    pinMode(PIN_BTN_D,  INPUT_PULLUP);
}

void Input::poll() {
    // Joystick axes  (0-4095 → -100..+100)
    // X is physically inverted per wiring doc
    int rawX = analogRead(PIN_JOY_X);
    int rawY = analogRead(PIN_JOY_Y);
    g_input.joyX = map(rawX, 0, 4095, 100, -100);   // inverted
    g_input.joyY = map(rawY, 0, 4095, -100, 100);

    // Buttons (active LOW)
    bool curJoy = !digitalRead(PIN_JOY_SW);
    bool curA   = !digitalRead(PIN_BTN_A);
    bool curB   = !digitalRead(PIN_BTN_B);
    bool curC   = !digitalRead(PIN_BTN_C);
    bool curD   = !digitalRead(PIN_BTN_D);

    g_input.joyBtn = curJoy;
    g_input.btnA   = curA;
    g_input.btnB   = curB;
    g_input.btnC   = curC;
    g_input.btnD   = curD;

    // Edge-detect (pressed THIS frame only)
    g_input.joyBtnP = curJoy && !_prevJoy;
    g_input.btnAP   = curA   && !_prevA;
    g_input.btnBP   = curB   && !_prevB;
    g_input.btnCP   = curC   && !_prevC;
    g_input.btnDP   = curD   && !_prevD;

    _prevJoy = curJoy;
    _prevA   = curA;
    _prevB   = curB;
    _prevC   = curC;
    _prevD   = curD;
}