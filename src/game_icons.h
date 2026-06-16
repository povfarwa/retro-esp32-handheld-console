#pragma once
#include <Arduino.h>
#include <stdint.h>

// Game Menu Icon Bitmaps (from actual device screenshots)
// RGB565 format, 36x36 pixels, stored in PROGMEM

struct IconInfo {
    const uint16_t* data;
    int16_t w;
    int16_t h;
    int16_t orig_w;
    int16_t orig_h;
};

extern const IconInfo gameIcons[6];
