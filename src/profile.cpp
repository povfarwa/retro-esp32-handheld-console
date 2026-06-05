#include "profile.h"
#include "ui.h"
#include "menu.h"
#include "settings.h"
#include "sounds.h"

// Name editing state
static int  nameLen     = 0;
static int  charIdx     = 0;   // cursor position in name
static bool editingName = false;
bool Profile::needsRedraw = true;

static const char CHARSET[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%&*";
static const int  CHARSET_LEN = 71;
static int        charSel = 0;

// ── Draw Name field ───────────────────────────────────────────────
static void drawNameRow() {
    int lx = INNER_X + 30;
    int ly = INNER_Y + 60;

    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString("NAME", lx, ly + 14);

    // Input box
    int bx = lx + 120, bw = 220, bh = 34;
    tft.fillRoundRect(bx, ly, bw, bh, 5, C_BG);
    tft.drawRoundRect(bx, ly, bw, bh, 5, editingName ? C_ACCENT : C_TEXT_DIM);

    // Name text
    tft.setTextColor(C_TEXT, C_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextSize(2);
    tft.drawString(g_app.playerName, bx + 8, ly + 18);

    if (editingName) {
        // Blinking cursor
        if ((millis() / 300) % 2 == 0) {
            int cx = bx + 8 + tft.textWidth(g_app.playerName, 2);
            tft.drawLine(cx, ly + 6, cx, ly + bh - 6, C_ACCENT);
        }
        // Character selector preview
        int px = bx + 8 + tft.textWidth(g_app.playerName, 2) + 4;
        if (px + 20 < bx + bw) {
            tft.fillRect(px - 2, ly + 4, 20, bh - 4, C_ACCENT);
            tft.setTextColor(C_WHITE, C_ACCENT);
            char preview[2] = { CHARSET[charSel], 0 };
            tft.drawString(preview, px, ly + 18);
        }
    }
}

// ── Draw Scores section ───────────────────────────────────────────
static void drawScores() {
    int lx = INNER_X + 30;
    int ly = INNER_Y + 110;

    // Title bar
    tft.fillRoundRect(lx, ly, INNER_W - 60, 24, 6, C_PANEL);
    tft.setTextColor(C_TEXT, C_PANEL);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.drawString("HIGH SCORES", lx + (INNER_W - 60) / 2, ly + 13);

    // Score lines for each game
    const char* gameNames[6] = {"SNAKE", "SPACE", "FLAPPY", "MAZE", "DINO", "RACER"};
    for (int i = 0; i < 6; i++) {
        int sy = ly + 32 + i * 18;
        tft.setTextColor(C_TEXT_DIM, C_BG);
        tft.setTextDatum(ML_DATUM);
        tft.setTextSize(1);
        tft.drawString(gameNames[i], lx + 10, sy + 4, 2);
        tft.setTextColor(C_TEXT, C_BG);
        char buf[16];
        sprintf(buf, "%lu", g_app.highScores[i]);
        tft.drawString(buf, lx + 160, sy + 4, 2);
    }
}

// ── Public API ────────────────────────────────────────────────────

void Profile::init() {
    nameLen     = strlen(g_app.playerName);
    editingName = false;
    charIdx     = nameLen;
    needsRedraw = true;
}

bool Profile::needsDraw() { return needsRedraw; }
void Profile::setNeedsRedraw() { needsRedraw = true; }

void Profile::draw() {
    if (!needsRedraw) return;
    needsRedraw = false;

    UI::drawChrome("PROFILE", NavActive::PROFILE);
    drawNameRow();
    drawScores();
}

void Profile::update() {
    if (editingName) {
        // Scroll through characters (up/down)
        if (g_input.joyY > 40 || g_input.btnBP) {
            charSel = (charSel + 1) % CHARSET_LEN;
            needsRedraw = true;
            delay(100);
        }
        if (g_input.joyY < -40 || g_input.btnCP) {
            charSel = (charSel - 1 + CHARSET_LEN) % CHARSET_LEN;
            needsRedraw = true;
            delay(100);
        }

        // Navigate within name (left/right)
        if (g_input.joyX > 40 || g_input.btnAP) {
            if (charIdx < nameLen) {
                charIdx++;
                needsRedraw = true;
                delay(100);
            }
        }
        if (g_input.joyX < -40 || g_input.btnDP) {
            if (charIdx > 0) {
                charIdx--;
                needsRedraw = true;
                delay(100);
            }
        }

        // Insert character at cursor position
        if (g_input.joyBtnP) {
            if (nameLen < 14) {
                for (int i = nameLen; i > charIdx; i--)
                    g_app.playerName[i] = g_app.playerName[i - 1];
                g_app.playerName[charIdx] = CHARSET[charSel];
                nameLen++;
                charIdx++;
                g_app.playerName[nameLen] = '\0';
                if (g_app.soundOn) Sounds::sfxClick();
                needsRedraw = true;
                delay(150);
            }
        }

        // Backspace
        if (g_input.btnDP && nameLen > 0 && charIdx > 0) {
            for (int i = charIdx - 1; i < nameLen; i++)
                g_app.playerName[i] = g_app.playerName[i + 1];
            nameLen--;
            charIdx--;
            needsRedraw = true;
            if (g_app.soundOn) Sounds::sfxBack();
            delay(150);
        }

        // Done editing
        if (g_input.btnAP) {
            editingName = false;
            needsRedraw = true;
            if (g_app.soundOn) Sounds::sfxSelect();
            delay(200);
        }
    } else {
        // Start editing name
        if (g_input.joyBtnP || g_input.btnAP) {
            editingName = true;
            charSel = 0;
            nameLen = strlen(g_app.playerName);
            charIdx = nameLen;
            needsRedraw = true;
            if (g_app.soundOn) Sounds::sfxClick();
            delay(200);
        }

        // Back to menu
        if (g_input.btnDP) {
            g_app.screen = Screen::HOME;
            if (g_app.soundOn) Sounds::sfxBack();
            Menu::setNeedsRedraw();
            delay(200);
        }

        // Settings shortcut
        if (g_input.btnBP) {
            g_app.prevScreen = Screen::PROFILE;
            g_app.screen = Screen::SETTINGS;
            if (g_app.soundOn) Sounds::sfxClick();
            Settings::setNeedsRedraw();
            delay(200);
        }
    }
}
