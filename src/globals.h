#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

// tft is defined in display.cpp
extern TFT_eSPI tft;

// ── Colour palette (RGB565) ──
#define C_BLACK      0x0000
#define C_WHITE      0xFFFF
#define C_RED        0xF800
#define C_GREEN      0x07E0
#define C_BLUE       0x001F
#define C_YELLOW     0xFFE0
#define C_CYAN       0x07FF
#define C_MAGENTA    0xF81F
#define C_ORANGE     0xFD20

// ── App state ──
struct AppState {
    bool     soundOn      = true;
    uint8_t  brightness   = 75;
    char     playerName[16] = "Player";
    uint32_t highScores[6]  = {0};
};
extern AppState g_app;

// ── Input snapshot ──
struct InputState {
    int16_t joyX, joyY;    // -100..+100
    bool    joyBtn;
    bool    btnA, btnB, btnC, btnD;
    bool    joyBtnP;
    bool    btnAP, btnBP, btnCP, btnDP;
};
extern InputState g_input;


