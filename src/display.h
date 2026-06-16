#pragma once
#include <TFT_eSPI.h>
#include "config.h"

// ─────────────────────────────────────────────
//  Display wrapper
//  Exposes tft (TFT_eSPI) + helper functions
// ─────────────────────────────────────────────

extern TFT_eSPI tft;

namespace Display {
    void init();
    void clear(uint16_t color = TFT_BLACK);

    // Centred text
    void drawCentredText(const char* text, int y, uint8_t size, uint16_t color);
    void drawCentredText(const String& text, int y, uint8_t size, uint16_t color);

    // Rounded panel with border
    void drawPanel(int x, int y, int w, int h,
                   uint16_t fillColor, uint16_t borderColor, int radius = 6);

    // Progress / health bar
    void drawBar(int x, int y, int w, int h,
                 float fraction, uint16_t fillColor, uint16_t bgColor = TFT_DARKGREY);
}
