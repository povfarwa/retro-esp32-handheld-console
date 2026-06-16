#include "settings.h"
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "globals.h"
#include "nvs_save.h"

namespace Settings {

static const uint16_t C_DARK_NAVY   = 0x0000;
static const uint16_t C_SLATE       = 0x8C71;
static const uint16_t C_NEON_BLUE   = 0x07FF;
static const uint16_t C_NEON_CYAN   = 0x06FF;
static const uint16_t C_PILL_GRAY   = 0x8C71;
static const uint16_t C_BLUE_GLOW   = 0x001F;
static const uint16_t C_CIRCUIT     = 0x00AA;

static int _cursor = 0;

void init() {
    NVS::load();
}

void save() {
    NVS::save();
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

    int batX = SCREEN_W - 195;
    tft.drawRect(batX, 6, 22, 12, TFT_BLACK);
    tft.drawRect(batX + 22, 9, 3, 6, TFT_BLACK);
    tft.fillRect(batX + 2, 8, 5, 8, C_NEON_BLUE);
    tft.setTextColor(TFT_WHITE, C_SLATE);
    tft.setCursor(batX + 28, 11);
    tft.print("25%");

    tft.setCursor(batX + 65, 11);
    tft.print("07:30 PM");

    int wifiX = batX + 105;
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

    tft.fillRoundRect(spacing - 30, SCREEN_H - 28, spacing + 60, 24, 6, C_DARK_NAVY);
    tft.setTextColor(C_NEON_BLUE, C_DARK_NAVY);
    tft.setTextSize(2);
    tft.setCursor(spacing + spacing/2 - 28, navY);
    tft.print("*");
    tft.setTextSize(1);
    tft.setCursor(spacing + spacing/2 - 12, navY + 4);
    tft.print("Settings");

    tft.setTextColor(TFT_WHITE, C_SLATE);
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
    tft.fillRoundRect(tx, 36, tw, 18, 9, C_PILL_GRAY);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_PILL_GRAY);
    tft.setCursor(tx + (tw - tft.textWidth("SETTINGS")) / 2, 39);
    tft.print("SETTINGS");
}

static void drawScreen() {
    tft.fillScreen(C_DARK_NAVY);
    drawTopBar();
    drawBottomBar();
    drawFrame();
    drawPageTitle();

    int labelX = 30;
    int valueX = 200;
    int yOff = 80;
    int lineH = 35;

    uint16_t cBright = (_cursor == 0) ? C_NEON_CYAN : TFT_WHITE;
    tft.setTextSize(2);
    tft.setTextColor(cBright, C_DARK_NAVY);
    tft.setCursor(labelX, yOff - 8);
    tft.print("BRIGHTNESS");

    int barX = valueX;
    int barY = yOff + 5;
    int barW = SCREEN_W - valueX - 30;
    int barH = 16;
    float frac = g_app.brightness / 100.0f;

    tft.fillRoundRect(barX, barY, barW, barH, 8, 0x2124);
    tft.fillRoundRect(barX, barY, (int)(barW * frac), barH, 8, C_NEON_BLUE);
    tft.fillCircle(barX + (int)(barW * frac), barY + barH/2, 8, TFT_WHITE);

    yOff += lineH + 15;
    uint16_t cSound = (_cursor == 1) ? C_NEON_CYAN : TFT_WHITE;
    tft.setTextSize(2);
    tft.setTextColor(cSound, C_DARK_NAVY);
    tft.setCursor(labelX, yOff);
    tft.print("SOUNDS");

    int swX = valueX;
    int swY = yOff;
    int swW = 60;
    int swH = 26;

    if (g_app.soundOn) {
        tft.fillRoundRect(swX, swY, swW, swH, 13, C_NEON_BLUE);
        tft.fillCircle(swX + swW - 13, swY + swH/2, 10, TFT_WHITE);
    } else {
        tft.fillRoundRect(swX, swY, swW, swH, 13, 0x630C);
        tft.fillCircle(swX + 13, swY + swH/2, 10, 0x3186);
    }

    tft.setTextSize(1);
    tft.setTextColor(cSound, C_DARK_NAVY);
    tft.setCursor(swX + swW + 10, swY + 6);
    tft.print(g_app.soundOn ? "ON" : "OFF");
}

void run() {
    _cursor = 0;
    drawScreen();

    while (true) {
        Input::update();

        Input::Axis ax = Input::axis();
        bool upPress   = Input::pressed(Input::TOP) || ax.y < -15;
        bool downPress = ax.y > 15;

        if (upPress && _cursor > 0) {
            _cursor--;
            Sounds::sfxClick();
            drawScreen();
        }
        if (downPress && _cursor < 1) {
            _cursor++;
            Sounds::sfxClick();
            drawScreen();
        }
        if (Input::pressed(Input::BOTTOM)) {
            if (_cursor == 1) {
                save();
                Sounds::sfxBack();
                return;
            } else if (_cursor < 1) {
                _cursor++;
                Sounds::sfxClick();
                drawScreen();
            }
        }

        if (Input::pressed(Input::SW)) {
            save();
            Sounds::sfxBack();
            return;
        }

        bool leftPressed = Input::pressed(Input::LEFT);
        bool rightPressed = Input::pressed(Input::RIGHT);

        if (leftPressed || rightPressed) {
            if (_cursor == 0) {

                if (rightPressed) {
                    g_app.brightness = (g_app.brightness + 10 > 100) ? 100 : g_app.brightness + 10;
                } else {
                    g_app.brightness = (g_app.brightness < 10) ? 0 : g_app.brightness - 10;
                }
                int pwm = map(g_app.brightness, 0, 100, 0, 255);
                analogWrite(PIN_BACKLIGHT, pwm);
                Sounds::sfxClick();
            }
            if (_cursor == 1) {

                if (rightPressed) {
                    if (!g_app.soundOn) {
                        g_app.soundOn = true;
                        Sounds::sfxClick();
                    }
                } else if (g_app.soundOn) {
                    g_app.soundOn = false;
                    Sounds::sfxClick();
                } else {

                    save();
                    Sounds::sfxBack();
                    return;
                }
            }
            drawScreen();
        }

        delay(16);
    }
}

}
