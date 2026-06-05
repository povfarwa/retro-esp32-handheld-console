#include "menu.h"
#include "ui.h"
#include "settings.h"
#include "profile.h"
#include "sounds.h"

// ── Tile layout ───────────────────────────────────────────────────
// 3 columns × 2 rows inside the inner content area

static const int COLS       = 3;
static const int ROWS       = 2;
static const int TILE_W     = 108;
static const int TILE_H     = 92;
static const int TILE_GAP_X = 12;
static const int TILE_GAP_Y = 10;

// Grid origin — centered within inner area, below title (ends at ~INNER_Y+50)
static const int GRID_Y     = INNER_Y + 56;
static const int GRID_TOTAL_W = COLS * TILE_W + (COLS - 1) * TILE_GAP_X;
static const int GRID_X     = INNER_X + (INNER_W - GRID_TOTAL_W) / 2;

static int sel = 0;
bool Menu::needsRedraw = true;

struct Tile {
    const char* name;   // "" = no label
    Screen      screen;
};

// ── 6 Game Tiles matching Canva design ──────────────────────────
// Only tile 0 ("BINGO") has a text label per the design
static const Tile TILES[6] = {
    { "BINGO",    Screen::GAME_SNAKE    },  // top-left:   BINGO card
    { "",         Screen::GAME_SPACE    },  // top-mid:    Potion flask
    { "",         Screen::GAME_FLAPPY   },  // top-right:  Hourglass
    { "",         Screen::GAME_PUZZLE   },  // bot-left:   Flame
    { "",         Screen::GAME_ASTEROID },  // bot-mid:    Cloud
    { "",         Screen::GAME_BRICK    },  // bot-right:  Chess knight
};

// ── Pixel art icons ──────────────────────────────────────────────

static void drawBingoCard(int cx, int cy, uint16_t c) {
    // BINGO card — a card with dots grid and "B" label
    tft.fillRoundRect(cx - 14, cy - 16, 28, 32, 3, C_PANEL);
    tft.drawRoundRect(cx - 14, cy - 16, 28, 32, 3, c);
    // B label at top
    tft.setTextColor(C_RED, C_PANEL);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(1);
    tft.drawString("B", cx, cy - 13);
    // 5×2 dot grid
    for (int r = 0; r < 2; r++) {
        for (int c2 = 0; c2 < 5; c2++) {
            int dx = cx - 10 + c2 * 5;
            int dy = cy - 4 + r * 10;
            tft.fillCircle(dx, dy, 1, c);
        }
    }
    // Bottom row of dots
    for (int c2 = 0; c2 < 4; c2++) {
        tft.fillCircle(cx - 8 + c2 * 5, cy + 8, 1, c);
    }
}

static void drawPotion(int cx, int cy, uint16_t c) {
    // Potion flask with liquid
    // Bottle body
    tft.fillRoundRect(cx - 9, cy - 4, 18, 22, 4, C_PANEL);
    tft.drawRoundRect(cx - 9, cy - 4, 18, 22, 4, c);
    // Neck
    tft.fillRect(cx - 5, cy - 14, 10, 12, C_PANEL);
    tft.drawRect(cx - 5, cy - 14, 10, 12, c);
    // Cork
    tft.fillRect(cx - 4, cy - 17, 8, 4, C_ORANGE);
    // Liquid inside
    tft.fillRect(cx - 7, cy + 6, 14, 10, C_GREEN);
    // Liquid surface shine
    tft.drawLine(cx - 6, cy + 6, cx + 6, cy + 6, C_WHITE);
    // Bubbles
    tft.fillCircle(cx - 2, cy + 10, 2, C_CYAN);
    tft.fillCircle(cx + 4, cy + 12, 1, C_YELLOW);
}

static void drawHourglass(int cx, int cy, uint16_t c) {
    // Hourglass
    int hw = 16, hh = 30;
    // Top triangle
    tft.drawLine(cx - hw / 2, cy - 15, cx + hw / 2, cy - 15, c);
    tft.drawLine(cx - hw / 2, cy - 15, cx, cy, c);
    tft.drawLine(cx + hw / 2, cy - 15, cx, cy, c);
    // Bottom triangle
    tft.drawLine(cx - hw / 2, cy + 15, cx + hw / 2, cy + 15, c);
    tft.drawLine(cx - hw / 2, cy + 15, cx, cy, c);
    tft.drawLine(cx + hw / 2, cy + 15, cx, cy, c);
    // Sand in bottom
    tft.fillTriangle(cx - 6, cy + 8, cx + 6, cy + 8, cx, cy + 4, C_YELLOW);
    // Sand falling (dotted line)
    tft.fillRect(cx - 1, cy + 1, 2, 5, C_YELLOW);
    // Top rim
    tft.drawLine(cx - hw / 2 - 2, cy - 15, cx + hw / 2 + 2, cy - 15, c);
    // Bottom rim
    tft.drawLine(cx - hw / 2 - 2, cy + 15, cx + hw / 2 + 2, cy + 15, c);
    // Frame sides
    tft.drawLine(cx - hw / 2 - 2, cy - 15, cx - hw / 2 - 2, cy + 15, c);
    tft.drawLine(cx + hw / 2 + 2, cy - 15, cx + hw / 2 + 2, cy + 15, c);
}

static void drawFlame(int cx, int cy, uint16_t c) {
    // Fireball / flame — layered teardrop
    // Outer flame (red)
    tft.fillTriangle(cx, cy - 18, cx - 12, cy + 6, cx + 12, cy + 6, C_RED);
    // Middle (orange)
    tft.fillTriangle(cx, cy - 12, cx - 7, cy + 6, cx + 7, cy + 6, C_ORANGE);
    // Inner core (yellow)
    tft.fillTriangle(cx, cy - 6, cx - 3, cy + 6, cx + 3, cy + 6, C_YELLOW);
    // Glow around base
    tft.fillCircle(cx, cy + 6, 8, C_RED);
    tft.fillCircle(cx, cy + 6, 5, C_ORANGE);
    tft.fillCircle(cx, cy + 6, 2, C_YELLOW);
    // Outline
    tft.drawLine(cx, cy - 18, cx - 12, cy + 6, c);
    tft.drawLine(cx, cy - 18, cx + 12, cy + 6, c);
    tft.drawArc(cx, cy + 6, 12, 8, 0, 180, c, C_BG);
}

static void drawCloud(int cx, int cy, uint16_t c) {
    // Cloud puff — overlapping circles
    tft.fillCircle(cx, cy + 2, 12, C_TEXT_DIM);
    tft.fillCircle(cx - 10, cy + 6, 8, C_TEXT_DIM);
    tft.fillCircle(cx + 10, cy + 6, 8, C_TEXT_DIM);
    tft.fillCircle(cx - 6, cy - 6, 7, C_TEXT_DIM);
    tft.fillCircle(cx + 6, cy - 4, 7, C_TEXT_DIM);
    tft.fillCircle(cx, cy - 8, 4, C_TEXT_DIM);
    // Outline
    tft.drawCircle(cx, cy + 2, 12, c);
    tft.drawCircle(cx - 10, cy + 6, 8, c);
    tft.drawCircle(cx + 10, cy + 6, 8, c);
    tft.drawCircle(cx - 6, cy - 6, 7, c);
    tft.drawCircle(cx + 6, cy - 4, 7, c);
}

static void drawChessKnight(int cx, int cy, uint16_t c) {
    // Chess knight piece — horse head on base
    int bw = 24, bh = 32;
    int bx = cx - bw / 2, by = cy - bh / 2;
    tft.fillRoundRect(bx, by, bw, bh, 4, C_PANEL);
    tft.drawRoundRect(bx, by, bw, bh, 4, c);
    // Horse head (simplified L-shape)
    tft.fillRect(cx - 4, by + 4, 8, 12, c);       // neck
    tft.fillRect(cx - 6, by + 6, 4, 6, c);         // neck left
    tft.fillCircle(cx + 2, by + 4, 6, c);          // head
    // Eye
    tft.fillCircle(cx + 4, by + 3, 2, C_BG);
    // Ear
    tft.fillTriangle(cx, by - 2, cx + 4, by - 8, cx + 6, by - 2, c);
    // Base rectangle
    tft.fillRect(cx - 8, by + bh - 8, 16, 6, c);
    // Base highlight
    tft.drawLine(cx - 6, by + bh - 4, cx + 6, by + bh - 4, C_TEXT_DIM);
}

// ── Draw a single tile ────────────────────────────────────────────
static void drawTile(int idx, bool selected) {
    int col = idx % COLS;
    int row = idx / COLS;
    int x   = GRID_X + col * (TILE_W + TILE_GAP_X);
    int y   = GRID_Y + row * (TILE_H + TILE_GAP_Y);

    uint16_t bg      = selected ? C_PANEL_SEL : C_PANEL;
    uint16_t outline = selected ? C_TILE_BORDER : C_PANEL;

    // Tile background
    tft.fillRoundRect(x, y, TILE_W, TILE_H, 12, bg);
    tft.drawRoundRect(x, y, TILE_W, TILE_H, 12, outline);

    // Selected glowing border (bright blue)
    if (selected) {
        for (int g = 1; g <= 3; g++) {
            uint16_t glow = (g == 1) ? C_TILE_BORDER : (g == 2) ? C_OUTLINE : C_ACCENT;
            tft.drawRoundRect(x - g, y - g, TILE_W + g * 2, TILE_H + g * 2, 12 + g, glow);
        }
    }

    // Icon area — centered in tile
    int cx = x + TILE_W / 2;
    int cy = y + TILE_H / 2 - 4;
    uint16_t ic = selected ? C_TEXT : C_TEXT_DIM;

    switch (idx) {
        case 0: drawBingoCard(cx, cy, ic);   break;
        case 1: drawPotion(cx, cy, ic);      break;
        case 2: drawHourglass(cx, cy, ic);   break;
        case 3: drawFlame(cx, cy, ic);       break;
        case 4: drawCloud(cx, cy, ic);       break;
        case 5: drawChessKnight(cx, cy, ic); break;
    }

    // Label (only tile 0 has "BINGO" per design)
    if (TILES[idx].name[0] != '\0') {
        tft.setTextColor(ic, bg);
        tft.setTextDatum(BC_DATUM);
        tft.setTextSize(1);
        tft.drawString(TILES[idx].name, cx, y + TILE_H - 6);
    }
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
