#include "menu.h"
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "game_icons.h"

namespace Menu {

static const uint16_t C_DARK_NAVY   = 0x0000;
static const uint16_t C_SLATE       = 0x8C71;
static const uint16_t C_NEON_BLUE   = 0x07FF;
static const uint16_t C_NEON_CYAN   = 0x06FF;
static const uint16_t C_CIRCUIT     = 0x00AA;
static const uint16_t C_PILL_GRAY   = 0x8C71;
static const uint16_t C_STATUS_BG   = 0x4A69;
static const uint16_t C_BATTERY_BLUE= 0x061F;
static const uint16_t C_WIFI_BLUE   = 0x061F;
static const uint16_t C_BLUE_GLOW   = 0x001F;

struct GameEntry {
    int         id;
    const char* name;
    int         iconIdx;
    const char* desc;
    uint16_t    color;
};

static const GameEntry _games[GAME_COUNT] = {
    { GAME_SNAKE,         "SNAKE",         0, "Eat & grow!",          TFT_CYAN     },
    { GAME_SPACEINVADERS, "INVADERS",      1, "Shoot the aliens!",    TFT_GREEN    },
    { GAME_PACMAN,        "PAC-MAN",      2, "Eat all the dots!",      TFT_YELLOW   },
    { GAME_BRICKBREAKER,  "BRICK",         3, "Break all bricks!",    TFT_ORANGE   },
    { GAME_DINORUN,       "DINO RUN",      4, "Jump over cacti!",     TFT_RED      },
    { GAME_TETRIS,        "TETRIS",        5, "Clear the lines!",     TFT_MAGENTA  },
};

#define MENU_SETTINGS (GAME_COUNT)
#define MENU_PROFILE  (GAME_COUNT + 1)
#define MENU_COUNT    (GAME_COUNT + 2)

static const char* _extraNames[2] = { "SETTINGS", "PROFILE" };
static const uint16_t _extraColors[2] = { C_NEON_BLUE, C_NEON_BLUE };

static int _selected = 0;
static int _prevSelected = -1;

static const int GRID_COLS = 3;
static const int GRID_ROWS = 2;
static const int GRID_X    = 18;
static const int GRID_Y    = 88;
static const int CELL_W    = 144;
static const int CELL_H    = 76;
static const int CELL_GAP  = 6;

static void drawCircuitPattern(int x, int y, int w, int h) {

    for (int yy = y; yy < y + h; yy += 16) {

        tft.drawPixel(x + 2, yy, C_CIRCUIT);
        tft.drawPixel(x + 2, yy + 4, C_CIRCUIT);
        tft.drawPixel(x + 6, yy + 4, C_CIRCUIT);
        tft.drawPixel(x + 6, yy + 8, C_CIRCUIT);
        tft.drawPixel(x + 2, yy + 8, C_CIRCUIT);

        tft.drawPixel(x + w - 2, yy, C_CIRCUIT);
        tft.drawPixel(x + w - 2, yy + 4, C_CIRCUIT);
        tft.drawPixel(x + w - 6, yy + 4, C_CIRCUIT);
        tft.drawPixel(x + w - 6, yy + 8, C_CIRCUIT);
        tft.drawPixel(x + w - 2, yy + 8, C_CIRCUIT);
    }
}

static void drawTopBar() {

    tft.fillRect(0, 0, SCREEN_W, 32, C_SLATE);

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_SLATE);
    tft.setCursor(8, 11);
    tft.print("STASIS GAMING");

    tft.drawRect(100, 9, 22, 12, C_BATTERY_BLUE);
    tft.drawRect(122, 12, 3, 6, C_BATTERY_BLUE);
    tft.fillRect(102, 11, 5, 8, C_BATTERY_BLUE);

    tft.setTextColor(C_BATTERY_BLUE, C_SLATE);
    tft.setCursor(128, 11);
    tft.print("25%");

    tft.setTextColor(TFT_WHITE, C_SLATE);
    int timeX = (SCREEN_W - tft.textWidth("07:30 PM")) / 2;
    tft.setCursor(timeX, 11);
    tft.print("07:30 PM");

    int wifiX = SCREEN_W - 100;
    tft.drawCircle(wifiX + 5, 15, 6, C_WIFI_BLUE);
    tft.drawCircle(wifiX + 5, 15, 3, C_WIFI_BLUE);
    tft.fillCircle(wifiX + 5, 15, 1, C_WIFI_BLUE);
    tft.setTextColor(C_WIFI_BLUE, C_SLATE);
    tft.setCursor(wifiX + 16, 7);
    tft.print("WiFi");
    tft.setCursor(wifiX + 16, 16);
    tft.print("(CONNECTED)");
}

static void drawBottomBar(int activeIndex) {
    tft.fillRect(0, SCREEN_H - 32, SCREEN_W, 32, C_SLATE);

    bool backActive = false;
    bool settingsActive = (activeIndex == MENU_SETTINGS);
    bool profileActive  = (activeIndex == MENU_PROFILE);

    int navY = SCREEN_H - 22;
    int spacing = SCREEN_W / 3;

    if (activeIndex == -99) backActive = false;
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, C_SLATE);
    tft.setCursor(spacing/2 - 20, navY);
    tft.print("<");
    tft.setTextSize(1);
    tft.setCursor(spacing/2 - 4, navY + 4);
    tft.print("Back");

    if (settingsActive) {
        tft.fillRoundRect(spacing - 30, SCREEN_H - 28, spacing + 60, 24, 6, C_DARK_NAVY);
        tft.setTextColor(C_NEON_BLUE, C_DARK_NAVY);
    } else {
        tft.setTextColor(TFT_WHITE, C_SLATE);
    }
    tft.setTextSize(2);
    tft.setCursor(spacing + spacing/2 - 28, navY);
    tft.print("*");
    tft.setTextSize(1);
    tft.setCursor(spacing + spacing/2 - 12, navY + 4);
    tft.print("Settings");

    if (profileActive) {
        tft.fillRoundRect(spacing * 2 - 30, SCREEN_H - 28, spacing + 60, 24, 6, C_DARK_NAVY);
        tft.setTextColor(C_NEON_BLUE, C_DARK_NAVY);
    } else {
        tft.setTextColor(TFT_WHITE, C_SLATE);
    }
    tft.setTextSize(2);
    tft.setCursor(spacing * 2 + spacing/2 - 24, navY);
    tft.print(")");
    tft.setTextSize(1);
    tft.setCursor(spacing * 2 + spacing/2 - 8, navY + 4);
    tft.print("Profile");
}

static void drawFrame() {

    for (int i = 0; i < 3; i++) {
        tft.drawFastVLine(4 + i, 32, SCREEN_H - 64, C_BLUE_GLOW);
    }

    for (int i = 0; i < 3; i++) {
        tft.drawFastVLine(SCREEN_W - 4 - i, 32, SCREEN_H - 64, C_BLUE_GLOW);
    }

    drawCircuitPattern(4, 36, 8, SCREEN_H - 72);
    drawCircuitPattern(SCREEN_W - 12, 36, 8, SCREEN_H - 72);
}

static void drawPageTitle() {
    int tw = 90;
    int tx = (SCREEN_W - tw) / 2;
    tft.fillRoundRect(tx, 38, tw, 18, 9, C_PILL_GRAY);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_PILL_GRAY);
    tft.setCursor(tx + (tw - tft.textWidth("HOME")) / 2, 41);
    tft.print("HOME");
}

static void drawCell(int index, bool isSelected) {
    bool isExtra = (index >= GAME_COUNT);
    int extraIdx = index - GAME_COUNT;
    int gridIdx  = index;

    if (isExtra) return;

    int row = gridIdx / GRID_COLS;
    int col = gridIdx % GRID_COLS;

    int cx = GRID_X + col * (CELL_W + CELL_GAP);
    int cy = GRID_Y + row * (CELL_H + CELL_GAP);

    const GameEntry& g = _games[gridIdx];

    uint16_t bgColor = C_SLATE;
    uint16_t borderColor = isSelected ? C_NEON_CYAN : 0x2124;
    uint16_t textColor = TFT_WHITE;

    if (isSelected) {
        tft.drawRoundRect(cx - 2, cy - 2, CELL_W + 4, CELL_H + 4, 10, C_NEON_CYAN);
        tft.drawRoundRect(cx - 1, cy - 1, CELL_W + 2, CELL_H + 2, 9, C_NEON_BLUE);
    } else {
        tft.fillRoundRect(cx - 2, cy - 2, CELL_W + 4, CELL_H + 4, 10, C_DARK_NAVY);
    }

    tft.fillRoundRect(cx, cy, CELL_W, CELL_H, 8, bgColor);
    tft.drawRoundRect(cx, cy, CELL_W, CELL_H, 8, borderColor);

    int iconSize = 32;
    int iconX = cx + (CELL_W - iconSize) / 2;
    int iconY = cy + 6;
    tft.fillRoundRect(iconX, iconY, iconSize, iconSize, 5, C_SLATE);

    const IconInfo& info = gameIcons[g.iconIdx];
    int imgX = iconX + (iconSize - info.w) / 2;
    int imgY = iconY + (iconSize - info.h) / 2;
    tft.pushImage(imgX, imgY, info.w, info.h, (uint16_t*)info.data, 0x0001);

    tft.setTextSize(1);
    tft.setTextColor(textColor, bgColor);
    tft.setCursor(cx + (CELL_W - tft.textWidth(g.name)) / 2, cy + 42);
    tft.print(g.name);

    tft.setTextSize(1);
    tft.setTextColor(0x2124, bgColor);
    tft.setCursor(cx + (CELL_W - tft.textWidth(g.desc)) / 2, cy + 54);
    tft.print(g.desc);
}

static void drawExtraCard(int index) {
    int extraIdx = index - GAME_COUNT;
    const char* name = _extraNames[extraIdx];
    const char* desc = (extraIdx == 0) ? "Sound, brightness" : "Your name & scores";
    uint16_t color = _extraColors[extraIdx];

    int cx = 80;
    int cy = 80;
    int cw = SCREEN_W - 160;
    int ch = 140;

    tft.drawRoundRect(cx - 2, cy - 2, cw + 4, ch + 4, 12, C_NEON_CYAN);

    tft.fillRoundRect(cx, cy, cw, ch, 10, color);
    tft.drawRoundRect(cx, cy, cw, ch, 10, TFT_WHITE);

    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, color);
    int16_t tw = tft.textWidth(name);
    tft.setCursor((SCREEN_W - tw) / 2, cy + 30);
    tft.print(name);

    tft.setTextSize(2);
    tft.setTextColor(TFT_NAVY, color);
    int16_t dw = tft.textWidth(desc);
    tft.setCursor((SCREEN_W - dw) / 2, cy + 75);
    tft.print(desc);

    tft.setTextColor(TFT_BLACK, C_DARK_NAVY);
    tft.setTextSize(3);
    if (index > GAME_COUNT) {
        tft.setCursor(30, cy + 50);
        tft.print("<");
    }
    if (index < MENU_COUNT - 1) {
        tft.setCursor(SCREEN_W - 50, cy + 50);
        tft.print(">");
    }
}

static void drawBackground() {
    tft.fillScreen(C_DARK_NAVY);

    drawTopBar();

    drawBottomBar(-1);

    drawFrame();

    drawPageTitle();
}

static void redraw() {
    drawBackground();

    for (int i = 0; i < GAME_COUNT; i++) {
        drawCell(i, i == _selected);
    }

    if (_selected >= GAME_COUNT) {
        drawExtraCard(_selected);
    }
}

void init() {
    _selected = 0;
}

int run() {
    _prevSelected = -1;
    redraw();
    _prevSelected = _selected;

    while (true) {
        Input::update();

        bool moved = false;

        if (Input::pressed(Input::LEFT)) {
            if (_selected > 0) {

                if (_selected < GAME_COUNT && _selected % GRID_COLS == 0) {

                    if (_selected >= GRID_COLS) _selected -= GRID_COLS;
                } else {
                    _selected--;
                }
                moved = true;
            }
        }
        if (Input::pressed(Input::RIGHT)) {
            if (_selected < MENU_COUNT - 1) {
                if (_selected < GAME_COUNT && _selected % GRID_COLS == GRID_COLS - 1) {

                    _selected += GRID_COLS;
                    if (_selected >= GAME_COUNT) _selected = GAME_COUNT - 1;
                } else {
                    _selected++;
                }
                moved = true;
            }
        }
        if (Input::pressed(Input::TOP)) {
            if (_selected >= GRID_COLS) {
                _selected -= GRID_COLS;
                moved = true;
            } else if (_selected >= GAME_COUNT) {
                _selected = GAME_COUNT - 1;
                moved = true;
            }
        }
        if (Input::pressed(Input::BOTTOM)) {
            if (_selected < GAME_COUNT - GRID_COLS) {
                _selected += GRID_COLS;
                moved = true;
            } else if (_selected < GAME_COUNT) {
                _selected = MENU_SETTINGS;
                moved = true;
            } else if (_selected < MENU_COUNT - 1) {
                _selected++;
                moved = true;
            }
        }

        if (moved) {
            Sounds::sfxClick();

            if (_prevSelected >= 0 && _prevSelected < GAME_COUNT) {
                drawCell(_prevSelected, false);
            }
            if (_selected >= 0 && _selected < GAME_COUNT) {
                drawCell(_selected, true);
            }

            if ((_prevSelected < GAME_COUNT) != (_selected < GAME_COUNT)) {
                drawBackground();
                for (int i = 0; i < GAME_COUNT; i++) {
                    drawCell(i, i == _selected);
                }
                if (_selected >= GAME_COUNT) drawExtraCard(_selected);
            }

            if (_prevSelected >= GAME_COUNT && _selected >= GAME_COUNT) {
                drawExtraCard(_selected);
            }

            drawBottomBar(_selected);

            _prevSelected = _selected;
        }

        if (Input::pressed(Input::SW)) {
            Sounds::sfxSelect();

            if (_selected < GAME_COUNT)
                return _games[_selected].id;
            else if (_selected == MENU_SETTINGS)
                return -2;
            else
                return -3;
        }

        delay(16);
    }
}

}
