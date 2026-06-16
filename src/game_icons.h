#pragma once
#include <Arduino.h>
#include <stdint.h>

struct IconInfo {
    const uint16_t* data;
    int16_t w;
    int16_t h;
    int16_t orig_w;
    int16_t orig_h;
};

extern const IconInfo gameIcons[6];
