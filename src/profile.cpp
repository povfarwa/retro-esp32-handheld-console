#include "profile.h"
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "globals.h"
#include "nvs_save.h"

namespace Profile {

static const uint16_t C_DARK_NAVY   = 0x0000;
static const uint16_t C_SLATE       = 0x8C71;
static const uint16_t C_NEON_BLUE   = 0x07FF;
static const uint16_t C_NEON_CYAN   = 0x06FF;
static const uint16_t C_PILL_GRAY   = 0x8C71;
static const uint16_t C_BLUE_GLOW   = 0x001F;
static const uint16_t C_CIRCUIT     = 0x00AA;

static const char* gameNames[GAME_COUNT] = {
    "Snake", "Space Shooter", "Pac-Man",
    "Maze Runner", "Dino Run", "Tetris"
};

void init() {
    NVS::load();
}

void setHighScore(int gameIdx, uint32_t score) {
    if (gameIdx >= 0 && gameIdx < GAME_COUNT && score > g_app.highScores[gameIdx]) {
        g_app.highScores[gameIdx] = score;
        NVS::save();
    }
}

uint32_t getHighScore(int gameIdx) {
    if (gameIdx >= 0 && gameIdx < GAME_COUNT)
        return g_app.highScores[gameIdx];
    return 0;
}

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

    tft.drawRect(100, 9, 22, 12, TFT_BLACK);
    tft.drawRect(122, 12, 3, 6, TFT_BLACK);
    tft.fillRect(102, 11, 5, 8, C_NEON_BLUE);
    tft.setCursor(128, 11);
    tft.print("25%");

    tft.setTextColor(TFT_WHITE, C_SLATE);
    int timeX = (SCREEN_W - tft.textWidth("07:30 PM")) / 2;
    tft.setCursor(timeX, 11);
    tft.print("07:30 PM");

    int wifiX = SCREEN_W - 100;
    tft.drawCircle(wifiX + 5, 15, 6, C_NEON_BLUE);
    tft.drawCircle(wifiX + 5, 15, 3, C_NEON_BLUE);
    tft.fillCircle(wifiX + 5, 15, 1, C_NEON_BLUE);
    tft.setTextColor(C_NEON_BLUE, C_SLATE);
    tft.setCursor(wifiX + 16, 7);
    tft.print("WiFi");
    tft.setCursor(wifiX + 16, 16);
    tft.print("(CONNECTED)");
}

static void drawBottomBar() {
    tft.fillRect(0, SCREEN_H - 32, SCREEN_W, 32, C_SLATE);

    int spacing = SCREEN_W / 3;
    int navY = SCREEN_H - 22;

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, C_SLATE);
    tft.setCursor(spacing/2 - 20, navY);
    tft.print("<");
    tft.setTextSize(1);
    tft.setCursor(spacing/2 - 4, navY + 4);
    tft.print("Back");

    tft.setTextColor(TFT_WHITE, C_SLATE);
    tft.setTextSize(2);
    tft.setCursor(spacing + spacing/2 - 28, navY);
    tft.print("*");
    tft.setTextSize(1);
    tft.setCursor(spacing + spacing/2 - 12, navY + 4);
    tft.print("Settings");

    tft.fillRoundRect(spacing * 2 - 30, SCREEN_H - 28, spacing + 60, 24, 6, C_DARK_NAVY);
    tft.setTextColor(C_NEON_BLUE, C_DARK_NAVY);
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
        tft.drawFastVLine(SCREEN_W - 4 - i, 32, SCREEN_H - 64, C_BLUE_GLOW);
    }
    drawCircuitPattern(4, 36, 8, SCREEN_H - 72);
    drawCircuitPattern(SCREEN_W - 12, 36, 8, SCREEN_H - 72);
}

static void drawPageTitle() {
    int tw = 110;
    int tx = (SCREEN_W - tw) / 2;
    tft.fillRoundRect(tx, 38, tw, 18, 9, C_PILL_GRAY);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_PILL_GRAY);
    tft.setCursor(tx + (tw - tft.textWidth("PROFILE")) / 2, 41);
    tft.print("PROFILE");
}

static void drawScreen() {
    tft.fillScreen(C_DARK_NAVY);
    drawTopBar();
    drawBottomBar();
    drawFrame();
    drawPageTitle();

    int cardX = 30;
    int cardY = 64;
    int cardW = SCREEN_W - 60;
    int cardH = 180;

    tft.drawRoundRect(cardX - 2, cardY - 2, cardW + 4, cardH + 4, 12, C_NEON_CYAN);
    tft.fillRoundRect(cardX, cardY, cardW, cardH, 10, 0x2124);
    tft.drawRoundRect(cardX, cardY, cardW, cardH, 10, C_NEON_BLUE);

    int labelX = cardX + 20;
    int valueX = cardX + 120;
    int row1 = cardY + 25;

    tft.setTextSize(2);
    tft.setTextColor(C_NEON_CYAN, 0x2124);
    tft.setCursor(labelX, row1);
    tft.print("NAME");

    int fw = cardW - 150;
    tft.fillRoundRect(valueX, row1 - 2, fw, 26, 6, C_DARK_NAVY);
    tft.drawRoundRect(valueX, row1 - 2, fw, 26, 6, C_NEON_BLUE);
    tft.setTextColor(TFT_WHITE, C_DARK_NAVY);
    tft.setTextSize(2);
    tft.setCursor(valueX + 10, row1 + 3);
    tft.print("FARWA ZAFAR");

    int row2 = row1 + 50;

    tft.setTextSize(2);
    tft.setTextColor(C_NEON_CYAN, 0x2124);
    tft.setCursor(labelX, row2);
    tft.print("AGE");

    tft.fillRoundRect(valueX, row2 - 2, 80, 26, 6, C_DARK_NAVY);
    tft.drawRoundRect(valueX, row2 - 2, 80, 26, 6, C_NEON_BLUE);
    tft.setTextColor(TFT_WHITE, C_DARK_NAVY);
    tft.setTextSize(2);
    tft.setCursor(valueX + 10, row2 + 3);
    tft.print("14");
}

void run() {

    tft.fillScreen(C_DARK_NAVY);
    drawScreen();

    while (true) {
        Input::update();

        if (Input::pressed(Input::LEFT) || Input::pressed(Input::SW)) {
            Sounds::sfxBack();
            return;
        }

        delay(16);
    }
}

}
