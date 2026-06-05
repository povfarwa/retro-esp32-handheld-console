#pragma once
#include "globals.h"

// Which nav item is "active" (highlighted)
enum class NavActive { BACK, SETTINGS, PROFILE };

namespace UI {
    // Draw the full chrome (status bar + side panels + nav bar)
    void drawChrome(const char* title, NavActive nav = NavActive::BACK);

    // Just redraw the title label (cheaper than full chrome)
    void drawTitle(const char* title);

    // Status bar only
    void drawStatusBar();

    // Nav bar only
    void drawNavBar(NavActive nav);

    // Side panels (circuit-board decorations)
    void drawSidePanels();

    // Rounded rectangle helper
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       int16_t r, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       int16_t r, uint16_t color);

    // Clear inner content area (between side panels, below status, above nav)
    void clearContent();
}