#include "settings.h"
#include "ui.h"
#include "menu.h"
#include "sounds.h"

// Focus: 0 = brightness slider, 1 = sound toggle
static int  focus       = 0;
bool Settings::needsRedraw = true;

// ── Apply brightness to backlight ─────────────────────────────────
// Uses LEDC PWM on TFT_BL pin (pin 21 from platformio.ini)
static void applyBrightness() {
    static bool pwmInit = false;
    if (!pwmInit) {
        ledcSetup(0, 5000, 8);      // channel 0, 5kHz, 8-bit
        ledcAttachPin(TFT_BL, 0);
        pwmInit = true;
    }
    uint8_t val = map(g_app.brightness, 0, 100, 0, 255);
    ledcWrite(0, val);
}

// ── Draw the brightness slider ────────────────────────────────────
static void drawBrightnessRow() {
    int lx = INNER_X + 30;
    int ly = INNER_Y + 90;

    // Label
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString("BRIGHTNESS", lx, ly + 6);

    // Slider track
    int sx = lx + 180, sy = ly + 6, sw = 190;
    tft.drawLine(sx, sy, sx + sw, sy, C_TEXT_DIM);

    // Thumb position based on brightness value
    int tx = sx + (int)((float)g_app.brightness / 100.0f * sw);
    tft.fillCircle(tx, sy, 7, focus == 0 ? C_ACCENT : C_TEXT_DIM);

    // Brightness percentage
    char buf[8]; sprintf(buf, "%d%%", g_app.brightness);
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextSize(1);
    tft.drawString(buf, sx + sw + 12, sy - 4);

    // Focus highlight
    if (focus == 0)
        tft.drawRoundRect(lx - 4, ly - 8, INNER_W - 22, 28, 6, C_ACCENT);
}

// ── Draw the sound toggle ─────────────────────────────────────────
static void drawSoundRow() {
    int lx = INNER_X + 30;
    int ly = INNER_Y + 148;

    // Label
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString("SOUNDS", lx, ly + 6);

    // Toggle pill
    int tx = lx + 180, ty = ly - 4;
    int tw = 56, th = 28;
    uint16_t pillBg = g_app.soundOn ? C_ACCENT : C_PANEL;
    tft.fillRoundRect(tx, ty, tw, th, th / 2, pillBg);

    // Thumb circle
    int thumbX = g_app.soundOn ? tx + tw - th / 2 - 2 : tx + th / 2 + 2;
    tft.fillCircle(thumbX, ty + th / 2, th / 2 - 3, C_WHITE);

    // "ON/OFF" label
    tft.setTextColor(C_TEXT_DIM, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString(g_app.soundOn ? "ON" : "OFF", tx + tw + 14, ly + 6);

    // Focus highlight
    if (focus == 1)
        tft.drawRoundRect(lx - 4, ly - 8, INNER_W - 22, 34, 6, C_ACCENT);
}

// ── Draw system info ──────────────────────────────────────────────
static void drawSysInfo() {
    int lx = INNER_X + 30;
    int ly = INNER_Y + 200;
    tft.setTextColor(C_TEXT_DIM, C_BG);
    tft.setTextSize(1);
    tft.drawString("Player: " + String(g_app.playerName), lx, ly, 2);
    char buf[16];
    sprintf(buf, "Uptime: %d min", (int)(millis() / 60000));
    tft.drawString(buf, lx, ly + 16, 2);
}

// ── Public API ────────────────────────────────────────────────────

void Settings::init() {
    focus = 0;
    needsRedraw = true;
    applyBrightness();
}

bool Settings::needsDraw() { return needsRedraw; }
void Settings::setNeedsRedraw() { needsRedraw = true; }

void Settings::draw() {
    if (!needsRedraw) return;
    needsRedraw = false;

    UI::drawChrome("SETTINGS", NavActive::SETTINGS);
    drawBrightnessRow();
    drawSoundRow();
    drawSysInfo();
}

void Settings::update() {
    bool changed = false;

    // Switch focus row (joystick Y or B/C buttons)
    if (g_input.joyY > 50 || g_input.btnBP) {
        focus = (focus + 1) % 2;
        changed = true;
        delay(200);
    }
    if (g_input.joyY < -50 || g_input.btnCP) {
        focus = (focus + 1) % 2;
        changed = true;
        delay(200);
    }

    if (focus == 0) {
        // Brightness adjustment
        if ((g_input.joyX > 40 || g_input.btnAP) && g_app.brightness < 100) {
            g_app.brightness = min(100, g_app.brightness + 5);
            changed = true;
            applyBrightness();
            delay(80);
        }
        if ((g_input.joyX < -40 || g_input.btnDP) && g_app.brightness > 0) {
            g_app.brightness = max(0, (int)g_app.brightness - 5);
            changed = true;
            applyBrightness();
            delay(80);
        }
    }

    if (focus == 1) {
        // Sound toggle on joystick push or A button
        if (g_input.joyBtnP || g_input.btnAP) {
            g_app.soundOn = !g_app.soundOn;
            changed = true;
            if (g_app.soundOn) Sounds::sfxSelect();
            delay(200);
        }
    }

    if (changed) needsRedraw = true;

    // Back to main menu (Btn D = left button or joystick press on focus 0)
    if (g_input.joyBtnP && focus == 0) {
        g_app.screen = Screen::HOME;
        if (g_app.soundOn) Sounds::sfxBack();
        Menu::setNeedsRedraw();
        delay(200);
    }

    if (g_input.btnDP) {
        g_app.screen = Screen::HOME;
        if (g_app.soundOn) Sounds::sfxBack();
        Menu::setNeedsRedraw();
        delay(200);
    }
}
