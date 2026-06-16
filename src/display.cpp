#include "display.h"
#include "config.h"

TFT_eSPI tft = TFT_eSPI();

namespace Display {

void init() {
    tft.init();
    tft.setRotation(1);   // Landscape 480×320
    tft.fillScreen(TFT_BLACK);

    // Backlight on
    pinMode(PIN_BACKLIGHT, OUTPUT);
    digitalWrite(PIN_BACKLIGHT, HIGH);
}

void clear(uint16_t color) {
    tft.fillScreen(color);
}

void drawCentredText(const char* text, int y, uint8_t size, uint16_t color) {
    tft.setTextSize(size);
    tft.setTextColor(color, TFT_BLACK);
    int16_t tw = tft.textWidth(text);
    tft.setCursor((SCREEN_W - tw) / 2, y);
    tft.print(text);
}

void drawCentredText(const String& text, int y, uint8_t size, uint16_t color) {
    drawCentredText(text.c_str(), y, size, color);
}

void drawPanel(int x, int y, int w, int h,
               uint16_t fillColor, uint16_t borderColor, int radius) {
    tft.fillRoundRect(x, y, w, h, radius, fillColor);
    tft.drawRoundRect(x, y, w, h, radius, borderColor);
}

void drawBar(int x, int y, int w, int h,
             float fraction, uint16_t fillColor, uint16_t bgColor) {
    fraction = constrain(fraction, 0.0f, 1.0f);
    tft.fillRect(x, y, w, h, bgColor);
    tft.fillRect(x, y, (int)(w * fraction), h, fillColor);
}

} // namespace Display
