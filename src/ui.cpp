#include "ui.h"
#include "sounds.h"

// ─── Helpers ─────────────────────────────────────────────────────

void UI::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       int16_t r, uint16_t color) {
    tft.fillRoundRect(x, y, w, h, r, color);
}

void UI::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       int16_t r, uint16_t color) {
    tft.drawRoundRect(x, y, w, h, r, color);
}

// ─── Status Bar ──────────────────────────────────────────────────
// Light slate-gray pill shape across top on dark background.
// Layout:  STASIS GAMING  [batt 25%]    07:30 PM    [WiFi (Connected)]
void UI::drawStatusBar() {
    int barX = 4, barY = 4, barW = SCREEN_W - 8, barH = STATUS_H - 8;
    int barR = barH / 2;

    // Main pill background
    tft.fillRoundRect(barX, barY, barW, barH, barR, C_STATUS_PILL);
    tft.drawRoundRect(barX, barY, barW, barH, barR, C_TEXT_DIM);

    int midY = barY + barH / 2;
    tft.setTextDatum(ML_DATUM);

    // ── "STASIS GAMING" (left) ──
    tft.setTextColor(C_TEXT, C_STATUS_PILL);
    tft.setTextSize(1);
    tft.drawString("STASIS GAMING", 14, midY + 2);

    // ── Battery capsule (left-centre) ──
    int battX = 170, battY = barY + 3, battW = 74, battH = barH - 6;
    int battR = battH / 2;
    tft.fillRoundRect(battX, battY, battW, battH, battR, C_PILL_DARK);
    tft.drawRoundRect(battX, battY, battW, battH, battR, C_TEXT_DIM);
    // Terminal nub
    tft.fillRect(battX + battW, battY + 5, 3, battH - 10, C_TEXT_DIM);

    // Blue battery outline icon (inside capsule)
    uint16_t battBlue = C_ACCENT;
    int icX = battX + 6, icY = midY;
    // Outline battery shape
    tft.drawRect(icX, icY - 5, 12, 10, battBlue);
    tft.drawRect(icX - 1, icY - 4, 1, 8, battBlue);  // left thick
    tft.drawRect(icX + 12, icY - 3, 2, 6, battBlue);  // terminal
    // Fill 3 bars for ~25%
    tft.fillRect(icX + 2, icY - 2, 2, 4, battBlue);
    tft.fillRect(icX + 5, icY - 2, 2, 4, battBlue);
    // 25% text
    tft.setTextColor(C_TEXT, C_PILL_DARK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("25%", icX + 18, midY + 1);

    // ── Time (center) ──
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_TEXT, C_STATUS_PILL);
    tft.setTextSize(2);
    // Fake time from millis
    uint32_t s   = millis() / 1000;
    uint32_t min_ = (s / 60) % 60;
    uint32_t hr  = (s / 3600) % 24;
    char buf[12];
    sprintf(buf, "%02d:%02d %s",
        hr % 12 == 0 ? 12 : hr % 12, min_, hr < 12 ? "AM" : "PM");
    tft.drawString(buf, SCREEN_W / 2, midY + 2);

    // ── WiFi capsule (right) ──
    int wifiX = SCREEN_W - 178, wifiY = barY + 3, wifiW = 172, wifiH = barH - 6;
    int wifiR = wifiH / 2;
    tft.fillRoundRect(wifiX, wifiY, wifiW, wifiH, wifiR, C_PILL_DARK);
    tft.drawRoundRect(wifiX, wifiY, wifiW, wifiH, wifiR, C_ACCENT);

    // WiFi arc icon (3 arcs)
    int wcX = wifiX + 20, wcY = midY;
    tft.fillCircle(wcX, wcY + 3, 2, C_ACCENT);
    tft.drawArc(wcX, wcY + 3, 5, 4, 210, 330, C_ACCENT, C_PILL_DARK);
    tft.drawArc(wcX, wcY + 3, 10, 8, 210, 330, C_ACCENT, C_PILL_DARK);
    tft.drawArc(wcX, wcY + 3, 15, 12, 210, 330, C_ACCENT, C_PILL_DARK);

    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(C_TEXT, C_PILL_DARK);
    tft.drawString("WiFi", wcX + 24, midY - 5);
    tft.setTextColor(C_TEXT_BLUE, C_PILL_DARK);
    tft.drawString("(Connected)", wcX + 24, midY + 7);
}

// ─── Title label ─────────────────────────────────────────────────
// Pill-shaped header centered at top of main container
void UI::drawTitle(const char* title) {
    int tw = 160, th = 34;
    int tx = (SCREEN_W - tw) / 2;
    int ty = INNER_Y + 8;

    // Horizontal connecting lines from side panels to title pill
    tft.drawLine(SIDE_W + 4, ty + th / 2, tx, ty + th / 2, C_TEXT_DIM);
    tft.drawLine(tx + tw, ty + th / 2, SCREEN_W - SIDE_W - 4, ty + th / 2, C_TEXT_DIM);

    // Title pill
    tft.fillRoundRect(tx, ty, tw, th, th / 2, C_LABEL_BG);
    tft.drawRoundRect(tx, ty, tw, th, th / 2, C_TEXT_DIM);

    tft.setTextColor(C_TEXT, C_LABEL_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString(title, SCREEN_W / 2, ty + th / 2 + 2);
}

// ─── Nav bar ─────────────────────────────────────────────────────
// Three items:  <- BACK  |  SETTINGS (gear)  |  PROFILE (person)
void UI::drawNavBar(NavActive nav) {
    // Background strip
    tft.fillRect(0, NAV_Y, SCREEN_W, NAV_H, C_NAV_BG);
    tft.drawLine(0, NAV_Y, SCREEN_W, NAV_Y, C_PANEL);

    const char* labels[3]  = {"BACK", "SETTINGS", "PROFILE"};
    int xs[3] = {0, SCREEN_W / 3, SCREEN_W * 2 / 3};
    int bw    = SCREEN_W / 3;
    int bh    = NAV_H - 6;
    int by_   = NAV_Y + 3;

    for (int i = 0; i < 3; i++) {
        bool active = (int)nav == i;
        uint16_t bg = active ? C_NAV_SEL : C_NAV_BG;
        uint16_t fg = active ? C_TEXT    : C_TEXT_DIM;

        if (active) {
            tft.fillRoundRect(xs[i] + 4, by_, bw - 8, bh, 8, bg);
            tft.drawRoundRect(xs[i] + 4, by_, bw - 8, bh, 8, C_ACCENT);
        }

        int cx = xs[i] + bw / 2;
        int cy = by_ + bh / 2;
        int ix = cx - 24;  // icon x offset

        if (i == 0) {
            // ← Back arrow (circle with left arrow)
            tft.drawCircle(ix + 2, cy, 9, fg);
            tft.drawLine(ix - 2, cy, ix + 7, cy, fg);
            tft.drawLine(ix - 2, cy, ix + 2, cy - 4, fg);
            tft.drawLine(ix - 2, cy, ix + 2, cy + 4, fg);
        } else if (i == 1) {
            // Gear (settings) - circle with outer teeth
            tft.fillCircle(ix + 2, cy, 5, fg);
            tft.drawCircle(ix + 2, cy, 5, fg);
            for (int a = 0; a < 360; a += 45) {
                float rad = a * 0.01745;
                int ox = (int)(cos(rad) * 9);
                int oy = (int)(sin(rad) * 9);
                tft.fillRect(ix + ox, cy + oy - 1, 3, 3, fg);
            }
        } else {
            // Person icon
            tft.fillCircle(ix + 2, cy - 5, 4, fg);
            tft.drawLine(ix - 3, cy + 9, ix + 2, cy - 1, fg);
            tft.drawLine(ix + 7, cy + 9, ix + 2, cy - 1, fg);
            tft.drawLine(ix - 3, cy + 9, ix + 7, cy + 9, fg);
        }

        // Label
        tft.setTextColor(fg, bg);
        tft.setTextDatum(ML_DATUM);
        tft.setTextSize(1);
        tft.drawString(labels[i], cx - 14, cy + 1);
    }
}

// ─── Side panels ─────────────────────────────────────────────────
void UI::drawSidePanels() {
    // Left panel
    tft.fillRect(0, STATUS_H, SIDE_W, INNER_H + 2, C_SIDE);
    tft.drawRect(0, STATUS_H, SIDE_W, INNER_H + 2, C_PANEL);

    // Right panel
    tft.fillRect(SCREEN_W - SIDE_W, STATUS_H, SIDE_W, INNER_H + 2, C_SIDE);
    tft.drawRect(SCREEN_W - SIDE_W, STATUS_H, SIDE_W, INNER_H + 2, C_PANEL);

    // Left blue accent bar
    tft.fillRect(SIDE_W - 8, STATUS_H + 20, 6, INNER_H - 40, C_ACCENT);
    // Chevrons on left panel
    for (int y_ = STATUS_H + 60; y_ < NAV_Y - 30; y_ += 30) {
        tft.drawLine(8,  y_,     18, y_ + 10, C_TEXT_DIM);
        tft.drawLine(18, y_ + 10, 8, y_ + 20, C_TEXT_DIM);
    }

    // Right blue accent bar
    tft.fillRect(SCREEN_W - SIDE_W + 2, STATUS_H + 20, 6, INNER_H - 40, C_ACCENT);
    for (int y_ = STATUS_H + 60; y_ < NAV_Y - 30; y_ += 30) {
        tft.drawLine(SCREEN_W - 8,  y_,     SCREEN_W - 18, y_ + 10, C_TEXT_DIM);
        tft.drawLine(SCREEN_W - 18, y_ + 10, SCREEN_W - 8, y_ + 20, C_TEXT_DIM);
    }

    // Circuit-board traces
    for (int y_ = STATUS_H + 30; y_ < NAV_Y - 10; y_ += 40) {
        tft.drawLine(4, y_, SIDE_W - 10, y_, C_TEXT_DIM);
        tft.drawLine(SCREEN_W - SIDE_W + 10, y_, SCREEN_W - 4, y_, C_TEXT_DIM);
        tft.fillCircle(SIDE_W - 10, y_, 2, C_TEXT_DIM);
        tft.fillCircle(SCREEN_W - SIDE_W + 10, y_, 2, C_TEXT_DIM);
    }
}

// ─── Clear content ────────────────────────────────────────────────
void UI::clearContent() {
    tft.fillRect(INNER_X, INNER_Y, INNER_W, INNER_H, C_BG);
}

// ─── Full chrome ─────────────────────────────────────────────────
void UI::drawChrome(const char* title, NavActive nav) {
    tft.fillScreen(C_BG);
    drawStatusBar();
    drawSidePanels();
    drawNavBar(nav);
    drawTitle(title);
}
