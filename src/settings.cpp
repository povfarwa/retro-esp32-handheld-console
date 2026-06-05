#include "settings.h"
#include "ui.h"
#include "menu.h"
#include "sounds.h"
#include "nvs_save.h"

// Focus: 0 = brightness slider, 1 = sound toggle
static int  focus       = 0;
bool Settings::needsRedraw = true;

// ── Apply brightness to backlight ─────────────────────────────────
static void applyBrightness() {
    static bool pwmInit = false;
    if (!pwmInit) {
        ledcSetup(0, 5000, 8);
        ledcAttachPin(TFT_BL, 0);
        pwmInit = true;
    }
    uint8_t val = map(g_app.brightness, 0, 100, 0, 255);
    ledcWrite(0, val);
}

// ── Draw the brightness slider ────────────────────────────────────
// Design: label "BRIGHTNESS" on left, slider bar to right.
// Bar: left portion up to the circular thumb is bright blue,
//      remaining right portion is gray.
static void drawBrightnessRow() {
    int lx = INNER_X + 32;
    int ly = INNER_Y + 85;

    // Label
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString("BRIGHTNESS", lx, ly + 6);

    // Slider bar
    int barX = lx + 210, barY = ly + 2, barW = 140, barH = 10;

    // Full track (gray)
    tft.fillRoundRect(barX, barY, barW, barH, barH / 2, C_PANEL);

    // Filled portion up to current value (bright blue)
    int fillW = (int)((float)g_app.brightness / 100.0f * (barW - 2));
    if (fillW > 0)
        tft.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, (barH - 2) / 2, C_ACCENT);

    // Circular thumb at transition point
    int thumbX = barX + fillW;
    int thumbY = barY + barH / 2;
    tft.fillCircle(thumbX, thumbY, 8, focus == 0 ? C_ACCENT : C_TEXT);
    tft.drawCircle(thumbX, thumbY, 8, C_WHITE);
    tft.fillCircle(thumbX, thumbY, 4, C_WHITE);

    // Brightness percentage label
    char buf[8];
    sprintf(buf, "%d%%", g_app.brightness);
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(1);
    tft.drawString(buf, barX + barW + 12, barY + barH / 2);

    // Focus highlight ring
    if (focus == 0)
        tft.drawRoundRect(lx - 4, ly - 10, INNER_W - 24, 34, 6, C_ACCENT);
}

// ── Draw the sound toggle ─────────────────────────────────────────
// Design: label "SOUNDS" on left, toggle switch to right.
// Switch: bright blue capsule with white circular slider + "ON/OFF" text
static void drawSoundRow() {
    int lx = INNER_X + 32;
    int ly = INNER_Y + 148;

    // Label
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString("SOUNDS", lx, ly + 6);

    // Toggle switch capsule
    int swX = lx + 210, swY = ly - 6, swW = 64, swH = 30;
    uint16_t capsuleBg = g_app.soundOn ? C_ACCENT : C_PANEL2;
    tft.fillRoundRect(swX, swY, swW, swH, swH / 2, capsuleBg);
    tft.drawRoundRect(swX, swY, swW, swH, swH / 2, C_ACCENT);

    // White circular slider knob
    int knobX = g_app.soundOn ? swX + swW - swH + 4 : swX + 4;
    tft.fillCircle(knobX, swY + swH / 2, swH / 2 - 5, C_WHITE);
    tft.drawCircle(knobX, swY + swH / 2, swH / 2 - 5, C_TEXT_DIM);
    // Inner dot
    tft.fillCircle(knobX, swY + swH / 2, 3, capsuleBg);

    // ON/OFF text
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString(g_app.soundOn ? "ON" : "OFF", swX + swW + 16, ly + 6);

    // Focus highlight ring
    if (focus == 1)
        tft.drawRoundRect(lx - 4, ly - 10, INNER_W - 24, 38, 6, C_ACCENT);
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
}

void Settings::update() {
    bool changed = false;

    // Switch focus row (joystick Y or B/C buttons)
    if (g_input.joyY > 50 || g_input.btnBP) {
        focus = (focus + 1) % 2;
        changed = true;
        if (g_app.soundOn) Sounds::sfxClick();
        delay(200);
    }
    if (g_input.joyY < -50 || g_input.btnCP) {
        focus = (focus + 1) % 2;
        changed = true;
        if (g_app.soundOn) Sounds::sfxClick();
        delay(200);
    }

    if (focus == 0) {
        // Brightness adjustment
        if ((g_input.joyX > 40 || g_input.btnAP) && g_app.brightness < 100) {
            g_app.brightness = min(100, g_app.brightness + 5);
            changed = true;
            applyBrightness();
            if (g_app.soundOn) Sounds::sfxClick();
            delay(80);
        }
        if ((g_input.joyX < -40 || g_input.btnDP) && g_app.brightness > 0) {
            g_app.brightness = max(0, (int)g_app.brightness - 5);
            changed = true;
            applyBrightness();
            if (g_app.soundOn) Sounds::sfxClick();
            delay(80);
        }
    }

    if (focus == 1) {
        // Toggle sound on joystick push or A button
        if (g_input.joyBtnP || g_input.btnAP) {
            g_app.soundOn = !g_app.soundOn;
            changed = true;
            if (g_app.soundOn) Sounds::sfxSelect();
            delay(200);
        }
    }

    if (changed) {
        needsRedraw = true;
        NVS::save();
    }

    // Back to main menu
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
