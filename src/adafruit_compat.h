#pragma once
#include <Arduino.h>
#include "globals.h"

#define FONT_W 6
#define FONT_H 8

static inline int textWidth(const char* s, int sz) {
    return strlen(s) * FONT_W * sz;
}
static inline int textWidth(const String& s, int sz) {
    return s.length() * FONT_W * sz;
}

static inline void drawCentreString(const char* s, int cx, int cy, int sz) {
    tft.setCursor(cx - strlen(s) * (FONT_W * sz / 2), cy - (FONT_H * sz / 2));
    tft.print(s);
}
static inline void drawCentreString(const String& s, int cx, int cy, int sz) {
    tft.setCursor(cx - s.length() * (FONT_W * sz / 2), cy - (FONT_H * sz / 2));
    tft.print(s);
}

static inline void drawStringML(const char* s, int x, int cy, int sz) {
    tft.setCursor(x, cy - (FONT_H * sz / 2));
    tft.print(s);
}
static inline void drawStringML(const String& s, int x, int cy, int sz) {
    tft.setCursor(x, cy - (FONT_H * sz / 2));
    tft.print(s);
}

static inline void drawStringMC(const char* s, int cx, int cy, int sz) {
    tft.setCursor(cx - strlen(s) * (FONT_W * sz / 2), cy - (FONT_H * sz / 2));
    tft.print(s);
}
static inline void drawStringMC(const String& s, int cx, int cy, int sz) {
    tft.setCursor(cx - s.length() * (FONT_W * sz / 2), cy - (FONT_H * sz / 2));
    tft.print(s);
}

static inline void drawStringTC(const char* s, int cx, int y_top, int sz) {
    tft.setCursor(cx - strlen(s) * (FONT_W * sz / 2), y_top);
    tft.print(s);
}
static inline void drawStringTC(const String& s, int cx, int y_top, int sz) {
    tft.setCursor(cx - s.length() * (FONT_W * sz / 2), y_top);
    tft.print(s);
}

static inline void drawStringBC(const char* s, int cx, int y_bot, int sz) {
    tft.setCursor(cx - strlen(s) * (FONT_W * sz / 2), y_bot - FONT_H * sz);
    tft.print(s);
}
static inline void drawStringBC(const String& s, int cx, int y_bot, int sz) {
    tft.setCursor(cx - s.length() * (FONT_W * sz / 2), y_bot - FONT_H * sz);
    tft.print(s);
}

static inline void drawFilledArc(int cx, int cy, int r_outer, int r_inner,
                                  int start_deg, int end_deg, uint16_t color) {

    if (start_deg > end_deg) end_deg += 360;
    for (int d = start_deg; d <= end_deg; d++) {
        float rad = d * 0.01745329252f;
        float c = cosf(rad);
        float s = sinf(rad);
        for (int rr = r_inner; rr <= r_outer; rr++) {
            tft.drawPixel((int)(cx + c * rr + 0.5f), (int)(cy + s * rr + 0.5f), color);
        }
    }
}
