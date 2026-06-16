#ifndef GAME_2048_H
#define GAME_2048_H

#include <Arduino.h>
#include <algorithm>
#include "config.h"
#include "globals.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "nvs_save.h"

namespace Game2048 {

// ── Constants ──
static const int GRID = 4;
static const int CELL = 100;
static const int GAP  = 8;
static const int GRID_W = GRID * CELL + (GRID + 1) * GAP;
static const int GRID_X = (SCREEN_W - GRID_W) / 2;
static const int GRID_Y = 55;

// ── Tile colors ──
static const uint16_t tileColors[] = {
    0xCDC4, // 2    - light beige
    0xCDC4, // 4    - light beige
    0xF7B0, // 8    - orange
    0xF7A0, // 16   - dark orange
    0xF770, // 32   - red-orange
    0xF750, // 64   - red
    0xF730, // 128  - bright red
    0xF710, // 256  - gold
    0xEE00, // 512  - yellow
    0xCC00, // 1024 - bright yellow
    0xAA00, // 2048 - gold
    0x8800, // 4096 - dark gold
    0x6600, // 8192 - brown
};

static const uint16_t tileTextColors[] = {
    0x7766, // 2    - dark brown
    0x7766, // 4    - dark brown
    TFT_WHITE, // 8
    TFT_WHITE, // 16
    TFT_WHITE, // 32
    TFT_WHITE, // 64
    TFT_WHITE, // 128
    TFT_WHITE, // 256
    TFT_WHITE, // 512
    TFT_WHITE, // 1024
    TFT_WHITE, // 2048
    TFT_WHITE, // 4096
    TFT_WHITE, // 8192
};

// ── State ──
static int board[GRID][GRID];
static int score;
static int highScore;
static bool gameOver;
static bool gameWon;
static bool gameStarted;
static int gameOverChoice;
static bool justMoved;

// ── Helpers ──
static int emptyCells() {
    int n = 0;
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++)
            if (board[r][c] == 0) n++;
    return n;
}

static void addTile() {
    if (emptyCells() == 0) return;
    int idx = random(0, emptyCells());
    for (int r = 0; r < GRID; r++) {
        for (int c = 0; c < GRID; c++) {
            if (board[r][c] == 0) {
                if (idx == 0) {
                    board[r][c] = (random(0, 10) == 0) ? 4 : 2;
                    return;
                }
                idx--;
            }
        }
    }
}

static int colorIndex(int val) {
    if (val == 0) return -1;
    int idx = 0;
    int v = val;
    while (v > 1 && idx < 12) { v >>= 1; idx++; }
    if (idx >= 13) idx = 12;
    return idx - 1;
}

// ── Slide & merge logic ──
static bool slideLeft() {
    bool moved = false;
    for (int r = 0; r < GRID; r++) {
        // Remove zeros
        int row[GRID], n = 0;
        for (int c = 0; c < GRID; c++)
            if (board[r][c] != 0) row[n++] = board[r][c];
        // Merge
        for (int i = 0; i < n - 1; i++) {
            if (row[i] == row[i + 1]) {
                row[i] *= 2;
                score += row[i];
                for (int j = i + 1; j < n - 1; j++) row[j] = row[j + 1];
                n--;
                moved = true;
            }
        }
        // Fill back
        for (int c = 0; c < n; c++) {
            if (board[r][c] != row[c]) moved = true;
            board[r][c] = row[c];
        }
        for (int c = n; c < GRID; c++) {
            if (board[r][c] != 0) moved = true;
            board[r][c] = 0;
        }
    }
    return moved;
}

static void rotateBoard() {
    int tmp[GRID][GRID];
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++)
            tmp[c][GRID - 1 - r] = board[r][c];
    memcpy(board, tmp, sizeof(board));
}

static bool slide(int dir) {
    // Rotate so we always slide left
    for (int i = 0; i < dir; i++) rotateBoard();
    bool moved = slideLeft();
    // Rotate back
    for (int i = 0; i < (4 - dir) % 4; i++) rotateBoard();
    return moved;
}

static bool canMerge() {
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++) {
            if (board[r][c] == 0) return true;
            if (c < GRID - 1 && board[r][c] == board[r][c + 1]) return true;
            if (r < GRID - 1 && board[r][c] == board[r + 1][c]) return true;
        }
    return false;
}

// ── Reset ──
static void reset() {
    memset(board, 0, sizeof(board));
    score = 0;
    gameOver = false;
    gameWon = false;
    justMoved = false;
    gameOverChoice = 0;
    addTile();
    addTile();
}

// ── Drawing ──
static void drawTile(int r, int c) {
    int x = GRID_X + GAP + c * (CELL + GAP);
    int y = GRID_Y + GAP + r * (CELL + GAP);
    int val = board[r][c];
    int ci = colorIndex(val);

    if (val == 0) {
        tft.fillRoundRect(x, y, CELL, CELL, 6, 0x3A2A);
        return;
    }

    uint16_t bg = tileColors[ci];
    uint16_t tc = tileTextColors[ci];

    tft.fillRoundRect(x, y, CELL, CELL, 6, bg);

    // Draw number
    tft.setTextSize(val < 100 ? 3 : (val < 1000 ? 2 : 1));
    tft.setTextColor(tc, bg);
    String s = String(val);
    int16_t tw = tft.textWidth(s.c_str());
    int16_t th = tft.fontHeight();
    tft.setCursor(x + (CELL - tw) / 2, y + (CELL - th) / 2);
    tft.print(s);
}

static void drawGrid() {
    // Background for the whole grid area
    tft.fillRoundRect(GRID_X, GRID_Y, GRID_W, GRID_W, 8, 0x2A1A);
    // Draw all cells
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++)
            drawTile(r, c);
}

static void drawHUD() {
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_BLACK);
    tft.setCursor(10, 8);
    tft.print("SCORE: ");
    tft.print(score);
    tft.setCursor(SCREEN_W - 80, 8);
    tft.print("BEST: ");
    tft.print(highScore);

    // Title
    tft.setTextSize(2);
    tft.setTextColor(0xF7B0, C_BLACK);
    int tw = tft.textWidth("2048");
    tft.setCursor((SCREEN_W - tw) / 2, 8);
    tft.print("2048");
}

// ── Game Over / Win ──
static void drawOverlay(const char* title, uint16_t color) {
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, color);

    tft.setTextSize(3);
    tft.setTextColor(color, C_BLACK);
    int tw = tft.textWidth(title);
    tft.setCursor((SCREEN_W - tw) / 2, 50);
    tft.print(title);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, C_BLACK);
    tft.setCursor(150, 100);
    tft.print("Score: ");
    tft.print(score);
    tft.setCursor(150, 125);
    tft.print("Best: ");
    tft.print(highScore);

    uint16_t colA = (gameOverChoice == 0) ? color : TFT_DARKGREY;
    uint16_t colB = (gameOverChoice == 1) ? color : TFT_DARKGREY;
    tft.setTextSize(2);
    tft.setTextColor(colA, C_BLACK);
    tft.setCursor(170, 175);
    tft.print("> Play Again");
    tft.setTextColor(colB, C_BLACK);
    tft.setCursor(170, 210);
    tft.print("  Back to Menu");
    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, C_BLACK);
    tft.setCursor(110, 260);
    tft.print("LEFT/RIGHT: select  SW: confirm");
}

// ── Run ──
void run() {
    gameStarted = false;
    gameOver = false;
    gameWon = false;
    gameOverChoice = 0;
    score = 0;
    highScore = g_app.highScores[2];

    tft.fillScreen(C_BLACK);

    // Title screen
    Display::drawPanel(40, 30, SCREEN_W - 80, 260, 0x3A2A, 0xF7B0, 12);

    // Draw a sample 4x4 grid on title
    int tx = 200, ty = 100;
    int ts = 20, tg = 3;
    int tgw = 4 * ts + 5 * tg;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            int v = (r * 4 + c + 1) % 2 == 0 ? 2 : 0;
            int cx = tx - tgw/2 + tg + c * (ts + tg);
            int cy = ty + tg + r * (ts + tg);
            tft.fillRoundRect(cx, cy, ts, ts, 3, v ? 0xCDC4 : 0x3A2A);
            if (v) {
                tft.setTextSize(1);
                tft.setTextColor(0x7766, 0xCDC4);
                tft.setCursor(cx + 6, cy + 5);
                tft.print("2");
            }
        }

    Display::drawCentredText("2048", 170, 3, 0xF7B0);
    Display::drawCentredText("Merge tiles to reach 2048!", 210, 2, TFT_WHITE);
    Display::drawCentredText("ARROWS: slide  SW: start", 245, 1, TFT_LIGHTGREY);
    Display::drawCentredText("Press SW to start", 270, 1, TFT_LIGHTGREY);
    Sounds::sfxSelect();

    while (true) {
        Input::update();
        if (Input::pressed(Input::SW)) {
            Input::beep(25); delay(150);
            gameStarted = true;
            reset();
            break;
        }
        delay(16);
    }

    // ── Game loop ──
    while (true) {
        Input::update();

        if (!gameOver && !gameWon) {
            if (Input::pressed(Input::SW)) {
                Sounds::sfxBack();
                return;
            }

            int dir = -1;
            if (Input::pressed(Input::LEFT))  dir = 2;
            else if (Input::pressed(Input::RIGHT)) dir = 0;
            else if (Input::pressed(Input::TOP))    dir = 3;
            else if (Input::pressed(Input::BOTTOM)) dir = 1;

            if (dir >= 0) {
                bool moved = slide(dir);
                if (moved) {
                    addTile();
                    Sounds::sfxClick();
                    justMoved = true;

                    // Check win
                    for (int r = 0; r < GRID; r++)
                        for (int c = 0; c < GRID; c++)
                            if (board[r][c] == 2048) {
                                gameWon = true;
                                highScore = max(highScore, score);
                                g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
                                NVS::save();
                                Sounds::sfxVictory();
                            }

                    // Check game over
                    if (!canMerge()) {
                        gameOver = true;
                        highScore = max(highScore, score);
                        g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
                        NVS::save();
                        Sounds::sfxGameOver();
                    }
                }
            }

            // Draw
            tft.fillScreen(C_BLACK);
            drawHUD();
            drawGrid();
        } else if (gameWon) {
            drawOverlay("YOU WIN!", 0xF7B0);
            bool chosen = false;
            while (!chosen) {
                Input::update();
                if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                    gameOverChoice = 1 - gameOverChoice;
                    Input::beep(25);
                    drawOverlay("YOU WIN!", 0xF7B0);
                }
                if (Input::pressed(Input::SW)) {
                    Input::beep(25); delay(150);
                    chosen = true;
                }
                if (Input::pressed(Input::TOP)) { delay(150); return; }
                delay(16);
            }
            if (gameOverChoice == 0) { reset(); gameWon = false; }
            else return;
        } else {
            drawOverlay("GAME OVER", 0xF800);
            bool chosen = false;
            while (!chosen) {
                Input::update();
                if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                    gameOverChoice = 1 - gameOverChoice;
                    Input::beep(25);
                    drawOverlay("GAME OVER", 0xF800);
                }
                if (Input::pressed(Input::SW)) {
                    Input::beep(25); delay(150);
                    chosen = true;
                }
                if (Input::pressed(Input::TOP)) { delay(150); return; }
                delay(16);
            }
            if (gameOverChoice == 0) reset();
            else return;
        }

        delay(16);
    }
}

} // namespace Game2048

#endif
