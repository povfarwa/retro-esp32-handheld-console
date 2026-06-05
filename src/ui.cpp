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

// ─── Status bar ──────────────────────────────────────────────────
// Matches: "STASIS GAMING  [batt 25%]  07:30 PM  [wifi] WiFi (Connected)"
void UI::drawStatusBar() {
    // Background pill
    tft.fillRoundRect(4, 4, SCREEN_W - 8, STATUS_H - 8, 12, C_PANEL);

    // Brand name
    tft.setTextColor(C_TEXT, C_PANEL);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(1);
    tft.setFreeFont(1);
    tft.setTextSize(2);
    tft.drawString("STASIS GAMING", 12, STATUS_H / 2);

    // Battery pill  (left-centre area)
    int bx = 190, by = 7, bw = 72, bh = 22;
    tft.fillRoundRect(bx, by, bw, bh, 5, C_PANEL);
    tft.drawRoundRect(bx, by, bw, bh, 5, C_TEXT_DIM);
    // Battery terminal nub
    tft.fillRect(bx + bw, by + 7, 4, 8, C_TEXT_DIM);
    // Fill level (25% example – will use actual later)
    uint8_t pct = 25;
    uint16_t fillColor = (pct < 20) ? C_RED : (pct < 50) ? C_YELLOW : C_GREEN;
    int fillW = max(2, (int)((bw - 6) * pct / 100));
    tft.fillRoundRect(bx + 3, by + 3, fillW, bh - 6, 3, fillColor);
    // Percent text
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_PANEL2);
    tft.setTextDatum(MC_DATUM);
    char buf[8]; sprintf(buf, "%d%%", pct);
    tft.drawString(buf, bx + bw / 2, by + bh / 2);

    // Time (centre of bar)
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_PANEL);
    tft.setTextDatum(MC_DATUM);

    // Get time string — use millis as fake clock for now
    // In real firmware swap with RTC or NTP
    uint32_t s   = millis() / 1000;
    uint32_t min_ = (s / 60) % 60;
    uint32_t hr  = (s / 3600) % 24;
    char timeBuf[12];
    sprintf(timeBuf, "%02d:%02d %s", hr % 12 == 0 ? 12 : hr % 12, min_, hr < 12 ? "AM" : "PM");
    tft.drawString(timeBuf, SCREEN_W / 2, STATUS_H / 2);

    // WiFi pill (right side)
    int wx = SCREEN_W - 170, wy = 5, ww = 164, wh = STATUS_H - 10;
    tft.fillRoundRect(wx, wy, ww, wh, 11, C_PANEL);
    tft.drawRoundRect(wx, wy, ww, wh, 11, C_ACCENT);

    // WiFi arc icon (simple 3-arc symbol)
    int ic = wx + 20, iy = wy + wh / 2;
    tft.drawCircle(ic, iy + 4, 2,  C_ACCENT);
    tft.drawArc   (ic, iy + 4, 6,  5, 210, 330, C_ACCENT, C_PANEL);
    tft.drawArc   (ic, iy + 4, 11, 9, 210, 330, C_ACCENT, C_PANEL);
    tft.drawArc   (ic, iy + 4, 16, 13,210, 330, C_ACCENT, C_PANEL);

    // WiFi label
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_PANEL);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("WiFi", wx + 32, iy - 4);
    tft.setTextColor(C_TEXT_BLUE, C_PANEL);
    tft.drawString("(Connected)", wx + 32, iy + 8);
}

// ─── Title label ─────────────────────────────────────────────────
// Rounded pill, centred above content, connected by thin lines to side panels
void UI::drawTitle(const char* title) {
    int tw  = 160, th = 34;
    int tx  = (SCREEN_W - tw) / 2;
    int ty  = INNER_Y + 8;

    // Horizontal connecting lines from side panels to title box
    tft.drawLine(SIDE_W, ty + th / 2, tx, ty + th / 2, C_TEXT_DIM);
    tft.drawLine(tx + tw, ty + th / 2, SCREEN_W - SIDE_W, ty + th / 2, C_TEXT_DIM);

    // Title pill
    tft.fillRoundRect(tx, ty, tw, th, 10, C_LABEL_BG);
    tft.drawRoundRect(tx, ty, tw, th, 10, C_TEXT_DIM);

    tft.setTextColor(C_TEXT, C_LABEL_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString(title, SCREEN_W / 2, ty + th / 2 + 1);
}

// ─── Nav bar ─────────────────────────────────────────────────────
// Three buttons: BACK | SETTINGS | PROFILE
void UI::drawNavBar(NavActive nav) {
    // Background strip
    tft.fillRect(0, NAV_Y, SCREEN_W, NAV_H, C_NAV_BG);
    tft.drawLine(0, NAV_Y, SCREEN_W, NAV_Y, C_PANEL);

    // Three nav items
    const char* labels[3]  = {"BACK", "SETTINGS", "PROFILE"};
    int xs[3] = {0, SCREEN_W / 3, SCREEN_W * 2 / 3};
    int bw     = SCREEN_W / 3;
    int bh     = NAV_H - 6;
    int by_    = NAV_Y + 3;

    for (int i = 0; i < 3; i++) {
        bool active = (int)nav == i;
        uint16_t bg  = active ? C_PANEL2 : C_NAV_BG;
        uint16_t fg  = active ? C_TEXT   : C_TEXT_DIM;

        if (active) {
            tft.fillRoundRect(xs[i] + 2, by_, bw - 4, bh, 8, bg);
            tft.drawRoundRect(xs[i] + 2, by_, bw - 4, bh, 8, C_ACCENT);
        }

        // Icon
        int cx = xs[i] + bw / 2;
        int cy = by_ + bh / 2;

        if (i == 0) {
            // Back arrow circle
            tft.drawCircle(cx - 24, cy, 9, fg);
            tft.drawLine(cx - 28, cy, cx - 19, cy,     fg);
            tft.drawLine(cx - 28, cy, cx - 24, cy - 4, fg);
            tft.drawLine(cx - 28, cy, cx - 24, cy + 4, fg);
        } else if (i == 1) {
            // Gear (settings) — simplified
            tft.drawCircle(cx - 22, cy, 6, fg);
            tft.drawCircle(cx - 22, cy, 3, fg);
            for (int a = 0; a < 360; a += 45) {
                float rad = a * 0.01745;
                int ox = (int)(cos(rad) * 9);
                int oy = (int)(sin(rad) * 9);
                tft.fillRect(cx - 22 + ox - 1, cy + oy - 1, 3, 3, fg);
            }
        } else {
            // Person icon
            tft.drawCircle(cx - 22, cy - 6, 4, fg);
            tft.drawLine(cx - 26, cy + 8, cx - 22, cy - 2, fg);
            tft.drawLine(cx - 18, cy + 8, cx - 22, cy - 2, fg);
            tft.drawLine(cx - 26, cy + 8, cx - 18, cy + 8, fg);
        }

        // Label
        tft.setTextColor(fg, active ? bg : C_NAV_BG);
        tft.setTextDatum(ML_DATUM);
        tft.setTextSize(1);
        tft.drawString(labels[i], cx - 12, cy + 1);
    }
}

// ─── Side panels ─────────────────────────────────────────────────
// Dark vertical strips with circuit-board line decoration + blue glow accent
void UI::drawSidePanels() {
    // Left panel
    tft.fillRect(0, STATUS_H, SIDE_W, INNER_H + 2, C_SIDE);
    tft.drawRect(0, STATUS_H, SIDE_W, INNER_H + 2, C_PANEL);

    // Right panel
    tft.fillRect(SCREEN_W - SIDE_W, STATUS_H, SIDE_W, INNER_H + 2, C_SIDE);
    tft.drawRect(SCREEN_W - SIDE_W, STATUS_H, SIDE_W, INNER_H + 2, C_PANEL);

    // Left blue accent bar
    tft.fillRect(SIDE_W - 8, STATUS_H + 20, 6, INNER_H - 40, C_ACCENT);
    // Arrow chevrons on left panel
    for (int y_ = STATUS_H + 60; y_ < NAV_Y - 30; y_ += 30) {
        tft.drawLine(8,  y_,     18, y_ + 10, C_TEXT_DIM);
        tft.drawLine(18, y_ + 10, 8, y_ + 20, C_TEXT_DIM);
    }

    // Right blue accent bar
    tft.fillRect(SCREEN_W - SIDE_W + 2, STATUS_H + 20, 6, INNER_H - 40, C_ACCENT);
    // Arrow chevrons on right panel (pointing right)
    for (int y_ = STATUS_H + 60; y_ < NAV_Y - 30; y_ += 30) {
        tft.drawLine(SCREEN_W - 8,  y_,     SCREEN_W - 18, y_ + 10, C_TEXT_DIM);
        tft.drawLine(SCREEN_W - 18, y_ + 10, SCREEN_W - 8, y_ + 20, C_TEXT_DIM);
    }

    // Circuit-board horizontal traces
    for (int y_ = STATUS_H + 30; y_ < NAV_Y - 10; y_ += 40) {
        tft.drawLine(4, y_, SIDE_W - 10, y_, C_TEXT_DIM);
        tft.drawLine(SCREEN_W - SIDE_W + 10, y_, SCREEN_W - 4, y_, C_TEXT_DIM);
        // Junction dots
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