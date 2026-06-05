#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// ── Display ──────────────────────────────────────────────────────
#define SCREEN_W     480
#define SCREEN_H     320

// Status bar (top strip)
#define STATUS_H     36
// Nav bar (bottom strip)
#define NAV_H        40
#define NAV_Y        (SCREEN_H - NAV_H)          // y=280
// Side panel strip width
#define SIDE_W       48
// Inner content area
#define INNER_X      SIDE_W                       // x=48
#define INNER_W      (SCREEN_W - SIDE_W * 2)     // w=384
#define INNER_Y      STATUS_H                     // y=36
#define INNER_H      (NAV_Y - STATUS_H)           // h=244

// ── GPIO ─────────────────────────────────────────────────────────
#define PIN_JOY_X    2
#define PIN_JOY_Y    1
#define PIN_JOY_SW   3
#define PIN_BTN_A    4   // RIGHT
#define PIN_BTN_B    5   // BOTTOM / SELECT
#define PIN_BTN_C    6   // TOP
#define PIN_BTN_D    7   // LEFT / BACK
#define PIN_BUZZER   46  // Buzzer/Haptic (NOT pin 8 — pin 8 is TFT_RST!)
#define TFT_BL       21  // Backlight PWM

// ── Colour palette  (RGB565) ─────────────────────────────────────
#define C_BLACK      0x0000
#define C_WHITE      0xFFFF
#define C_RED        0xF800
#define C_GREEN      0x07E0
#define C_BLUE       0x001F
#define C_YELLOW     0xFFE0
#define C_CYAN       0x07FF
#define C_MAGENTA    0xF81F
#define C_ORANGE     0xFD20

#define C_BG         0x0848   // #0C1020 very dark slate-gray (main bg)
#define C_SIDE       0x0A1A   // slightly lighter for side panels
#define C_PANEL      0x1C4A   // steel blue-grey for tiles/cards
#define C_PANEL2     0x1830   // darker panel for nav items
#define C_PANEL_SEL  0x0C2F   // selected tile fill (darkened)
#define C_STATUS_PILL 0x1C4A  // light slate-gray pill for status bar
#define C_PILL_DARK  0x0A18   // dark capsule for battery/wifi indicators
#define C_OUTLINE    0x051F   // electric blue outline
#define C_ACCENT     0x07BF   // cyan accent (WiFi icon, glow, borders)
#define C_LABEL_BG   0x1C4A   // rounded label background (HOME title)
#define C_TEXT       0xFFFF   // white
#define C_TEXT_DIM   0x8C71   // grey text
#define C_TEXT_BLUE  0x05FF   // light cyan for "Connected"
#define C_TILE_BORDER 0x033F  // bright blue border on selected tile
#define C_NAV_BG     0x0A1A   // nav bar background
#define C_NAV_SEL    0x1830   // selected nav item darker pill

// ── App screens ───────────────────────────────────────────────────
enum class Screen {
    HOME,
    SETTINGS,
    PROFILE,
    GAME_FLAPPY,
    GAME_BRICK,
    GAME_SPACE,
    GAME_PUZZLE,
    GAME_SNAKE,
    GAME_ASTEROID
};

struct AppState {
    Screen   screen       = Screen::HOME;
    Screen   prevScreen   = Screen::HOME;
    bool     soundOn      = true;
    uint8_t  brightness   = 75;          // 0-100
    char     playerName[16] = "Player";
    uint32_t highScores[6]  = {0};
};
extern AppState g_app;

// ── Input snapshot ────────────────────────────────────────────────
struct InputState {
    int16_t joyX;           // -100..+100
    int16_t joyY;           // -100..+100
    bool    joyBtn;
    bool    btnA, btnB, btnC, btnD;
    // one-shot press flags (set for exactly one frame)
    bool    joyBtnP;
    bool    btnAP, btnBP, btnCP, btnDP;
};
extern InputState g_input;

// ── Game phase ────────────────────────────────────────────────────
enum class GameState { PLAYING, PAUSED, LEVEL_UP, GAME_OVER };
