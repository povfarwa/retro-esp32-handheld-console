#pragma once
#include <TFT_eSPI.h>
#include "config.h"

extern TFT_eSPI tft;

namespace Display {
    void init();
    void clear(uint16_t color = TFT_BLACK);

    void drawCentredText(const char* text, int y, uint8_t size, uint16_t color);
    void drawCentredText(const String& text, int y, uint8_t size, uint16_t color);

    void drawPanel(int x, int y, int w, int h,
                   uint16_t fillColor, uint16_t borderColor, int radius = 6);

    void drawBar(int x, int y, int w, int h,
                 float fraction, uint16_t fillColor, uint16_t bgColor = TFT_DARKGREY);
}
