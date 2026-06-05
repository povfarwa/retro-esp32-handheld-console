#include "menu.h"
#include "ui.h"
#include "settings.h"
#include "profile.h"
#include "sounds.h"

// ── Tile layout ───────────────────────────────────────────────────
// 3 columns × 2 rows inside the inner content area, below the title label

static const int COLS       = 3;
static const int ROWS       = 2;
static const int TILE_W     = 108;
static const int TILE_H     = 88;
static const int TILE_GAP_X = 12;
static const int TILE_GAP_Y = 10;

// Grid origin — centered within inner area, below title (which ends at ~INNER_Y+50)
static const int GRID_Y     = INNER_Y + 54;
static const int GRID_TOTAL_W = COLS * TILE_W + (COLS - 1) * TILE_GAP_X;
static const int GRID_X     = INNER_X + (INNER_W - GRID_TOTAL_W) / 2;

static int sel = 0;
bool Menu::needsRedraw = true;

struct Tile {
    const char* name;
    Screen      screen;
};

// ── 6 Retro Games ────────────────────────────────────────────────
static const Tile TILES[6] = {
    { "SNAKE",    Screen::GAME_SNAKE   },   // top-left
    { "SPACE",    Screen::GAME_SPACE   },   // top-mid
    { "FLAPPY",   Screen::GAME_FLAPPY  },   // top-right
    { "MAZE",     Screen::GAME_PUZZLE  },   // bot-left
    { "DINO",     Screen::GAME_ASTEROID},   // bot-mid
    { "RACER",    Screen::GAME_BRICK   },   // bot-right
};

// ── Draw a single tile ────────────────────────────────────────────
static void drawTile(int idx, bool selected) {
    int col = idx % COLS;
    int row = idx / COLS;
    int x   = GRID_X + col * (TILE_W + TILE_GAP_X);
    int y   = GRID_Y + row * (TILE_H + TILE_GAP_Y);

    uint16_t bg      = selected ? C_PANEL_SEL : C_PANEL;
    uint16_t outline = selected ? C_OUTLINE   : C_PANEL;

    tft.fillRoundRect(x, y, TILE_W, TILE_H, 12, bg);
    tft.drawRoundRect(x, y, TILE_W, TILE_H, 12, outline);
    if (selected) {
        tft.drawRoundRect(x - 1, y - 1, TILE_W + 2, TILE_H + 2, 13, C_ACCENT);
    }

    // Icon area — centered in tile
    int cx = x + TILE_W / 2;
    int cy = y + TILE_H / 2 - 6;
    uint16_t ic = C_TEXT_DIM;

    switch (idx) {
        case 0: { // SNAKE — wavy serpent
            tft.drawCircle(cx, cy - 4, 10, ic);
            tft.fillCircle(cx, cy - 4, 4, C_GREEN);
            tft.drawLine(cx - 6, cy + 6, cx - 2, cy + 2, ic);
            tft.drawLine(cx - 2, cy + 2, cx + 2, cy + 6, ic);
            tft.drawLine(cx + 2, cy + 6, cx + 6, cy + 2, ic);
            tft.fillCircle(cx - 10, cy - 4, 2, C_RED); // food
            break;
        }
        case 1: { // SPACE — rocket ship
            tft.fillTriangle(cx, cy - 18, cx - 12, cy + 10, cx + 12, cy + 10, C_CYAN);
            tft.fillRect(cx - 4, cy + 10, 8, 8, C_RED);
            tft.fillTriangle(cx - 4, cy + 18, cx + 4, cy + 18, cx, cy + 26, C_YELLOW);
            tft.fillCircle(cx - 18, cy - 8, 2, C_WHITE);
            tft.fillCircle(cx + 16, cy - 12, 2, C_WHITE);
            tft.fillCircle(cx + 20, cy + 4, 2, C_WHITE);
            break;
        }
        case 2: { // FLAPPY — bird
            tft.fillCircle(cx - 4, cy - 4, 8, C_YELLOW);
            tft.fillCircle(cx + 6, cy - 6, 3, C_BLACK);
            tft.fillTriangle(cx + 4, cy - 2, cx + 12, cy - 2, cx + 8, cy + 4, C_ORANGE);
            tft.fillTriangle(cx - 6, cy + 4, cx - 2, cy + 10, cx + 2, cy + 4, C_YELLOW);
            break;
        }
        case 3: { // MAZE — grid with path
            tft.drawRect(cx - 16, cy - 16, 32, 32, ic);
            tft.drawLine(cx - 16, cy - 8, cx, cy - 8, ic);
            tft.drawLine(cx + 8, cy - 16, cx + 8, cy, ic);
            tft.drawLine(cx, cy, cx + 16, cy, ic);
            tft.drawLine(cx - 8, cy, cx - 8, cy + 16, ic);
            tft.drawLine(cx + 8, cy + 8, cx + 16, cy + 8, ic);
            tft.fillCircle(cx - 12, cy - 12, 4, C_GREEN);
            tft.fillCircle(cx + 12, cy + 12, 4, C_RED);
            break;
        }
        case 4: { // DINO — T-Rex
            tft.fillRect(cx - 8, cy - 2, 16, 16, C_GREEN);
            tft.fillRect(cx + 6, cy - 8, 10, 10, C_GREEN);
            tft.fillCircle(cx + 12, cy - 5, 2, C_WHITE);
            tft.fillRect(cx - 6, cy + 14, 4, 6, C_GREEN);
            tft.fillRect(cx + 2, cy + 14, 4, 6, C_GREEN);
            tft.fillRect(cx - 14, cy + 2, 6, 4, C_GREEN);
            break;
        }
        case 5: { // RACER — car
            tft.fillRoundRect(cx - 16, cy - 6, 32, 18, 4, C_RED);
            tft.fillRoundRect(cx - 8, cy - 12, 16, 10, 3, C_BLUE);
            tft.fillRect(cx - 6, cy - 10, 5, 6, C_CYAN);
            tft.fillRect(cx + 1, cy - 10, 5, 6, C_CYAN);
            tft.fillCircle(cx - 8, cy + 12, 5, C_BLACK);
            tft.fillCircle(cx + 8, cy + 12, 5, C_BLACK);
            tft.fillCircle(cx - 14, cy + 2, 2, C_YELLOW);
            tft.fillCircle(cx + 14, cy + 2, 2, C_YELLOW);
            break;
        }
    }

    // Game name label at bottom of tile
    tft.setTextColor(ic, bg);
    tft.setTextDatum(BC_DATUM);
    tft.setTextSize(1);
    tft.drawString(TILES[idx].name, cx, y + TILE_H - 6);
}

// ── Public API ────────────────────────────────────────────────────

void Menu::init() {
    sel = 0;
    needsRedraw = true;
}

bool Menu::needsDraw() { return needsRedraw; }

void Menu::setNeedsRedraw() { needsRedraw = true; }

void Menu::draw() {
    if (!needsRedraw) return;
    needsRedraw = false;

    UI::drawChrome("HOME", NavActive::BACK);

    for (int i = 0; i < 6; i++)
        drawTile(i, i == sel);
}

void Menu::update() {
    bool changed = false;

    // Joystick navigation
    if (g_input.joyX > 40 && sel % COLS < COLS - 1) { sel++; changed = true; delay(150); }
    if (g_input.joyX < -40 && sel % COLS > 0)        { sel--; changed = true; delay(150); }
    if (g_input.joyY > 40  && sel + COLS < 6)         { sel += COLS; changed = true; delay(150); }
    if (g_input.joyY < -40 && sel - COLS >= 0)        { sel -= COLS; changed = true; delay(150); }

    // D-pad buttons
    if (g_input.btnAP && sel % COLS < COLS - 1) { sel++; changed = true; delay(150); }
    if (g_input.btnDP && sel % COLS > 0)         { sel--; changed = true; delay(150); }
    if (g_input.btnBP && sel + COLS < 6)         { sel += COLS; changed = true; delay(150); }
    if (g_input.btnCP && sel - COLS >= 0)        { sel -= COLS; changed = true; delay(150); }

    if (changed) {
        if (g_app.soundOn) Sounds::sfxClick();
        needsRedraw = true;
    }

    // Select game — joystick press or btnA
    if (g_input.joyBtnP || g_input.btnAP) {
        if (g_app.soundOn) Sounds::sfxSelect();
        g_app.prevScreen = Screen::HOME;
        g_app.screen     = TILES[sel].screen;
        delay(200);
    }

    // Settings (BtnB / bottom button)
    if (g_input.btnBP) {
        g_app.prevScreen = Screen::HOME;
        g_app.screen     = Screen::SETTINGS;
        if (g_app.soundOn) Sounds::sfxClick();
        Settings::setNeedsRedraw();
        delay(200);
    }

    // Profile (BtnC / top button)
    if (g_input.btnCP && !changed) {
        g_app.prevScreen = Screen::HOME;
        g_app.screen     = Screen::PROFILE;
        if (g_app.soundOn) Sounds::sfxClick();
        Profile::setNeedsRedraw();
        delay(200);
    }
}
