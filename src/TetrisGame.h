#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "globals.h"
#include "nvs_save.h"

namespace Tetris {

// ── Constants ──────────────────────────────────
static const int COLS      = 10;
static const int ROWS      = 20;
static const int BSIZE     = 14;        // block size in pixels
static const int GRID_W    = COLS * BSIZE;   // 140
static const int GRID_H    = ROWS * BSIZE;   // 280
static const int GRID_X    = (SCREEN_W - GRID_W) / 2;  // 170
static const int GRID_Y    = 20;

static const int SIDE_X    = GRID_X + GRID_W + 8;   // right panel

// ── Tetromino definitions ─────────────────────
// 7 pieces × 4 rotations × 4×4 grid = each encoded as 4 bytes (rows)
// Pieces: I, O, T, S, Z, J, L
static const uint8_t PIECES[7][4][4] = {
    { // I
        {0,15,0,0},
        {2,2,2,2},
        {0,0,15,0},
        {4,4,4,4},
    },
    { // O
        {0,6,6,0},
        {0,6,6,0},
        {0,6,6,0},
        {0,6,6,0},
    },
    { // T
        {0,4,14,0},
        {4,6,4,0},
        {0,14,4,0},
        {4,12,4,0},
    },
    { // S
        {0,6,12,0},
        {4,6,2,0},
        {0,6,12,0},
        {4,6,2,0},
    },
    { // Z
        {0,12,6,0},
        {2,6,4,0},
        {0,12,6,0},
        {2,6,4,0},
    },
    { // J
        {0,8,14,0},
        {6,4,4,0},
        {0,14,2,0},
        {4,4,12,0},
    },
    { // L
        {0,2,14,0},
        {4,4,6,0},
        {0,14,8,0},
        {12,4,4,0},
    },
};

static const uint16_t PIECE_COLORS[7] = {
    TFT_CYAN,   // I
    TFT_YELLOW, // O
    TFT_MAGENTA,// T
    TFT_GREEN,  // S
    TFT_RED,    // Z
    TFT_BLUE,   // J
    TFT_ORANGE  // L
};

// ── State ──────────────────────────────────────
static uint8_t grid[ROWS][COLS];   // 0 = empty, 1-7 = piece color index+1
static int currentPiece;            // 0-6
static int currentRot;              // 0-3
static int px, py;                  // top-left of 4×4 bounding box on grid
static int nextPiece;
static int score;
static int level;
static int lines;
static bool isGameOver;
static unsigned long lastFall;
static int fallDelay;               // ms per drop
static bool gameStarted;
static int gameOverChoice;

// ── Helpers ────────────────────────────────────

static void drawBlock(int col, int row, uint16_t color) {
    int x = GRID_X + col * BSIZE;
    int y = GRID_Y + row * BSIZE;
    tft.fillRect(x + 1, y + 1, BSIZE - 2, BSIZE - 2, color);
    // Slight 3D effect
    tft.drawRect(x, y, BSIZE, BSIZE, TFT_BLACK);
}

static void clearBlock(int col, int row) {
    int x = GRID_X + col * BSIZE;
    int y = GRID_Y + row * BSIZE;
    tft.fillRect(x, y, BSIZE, BSIZE, TFT_BLACK);
}

static bool isOccupied(int col, int row) {
    if (col < 0 || col >= COLS) return true;
    if (row >= ROWS) return true;
    if (row < 0) return false;
    return grid[row][col] != 0;
}

static bool pieceFits(int piece, int rot, int x, int y) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (PIECES[piece][rot][r] & (1 << (3 - c))) {
                int cx = x + c;
                int cy = y + r;
                if (cx < 0 || cx >= COLS) return false;
                if (cy >= ROWS) return false;
                if (cy >= 0 && grid[cy][cx] != 0) return false;
            }
        }
    }
    return true;
}

static void drawPiece(int piece, int rot, int x, int y, bool erase) {
    uint16_t color = erase ? TFT_BLACK : PIECE_COLORS[piece];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (PIECES[piece][rot][r] & (1 << (3 - c))) {
                int cx = x + c;
                int cy = y + r;
                if (cy >= 0) {
                    if (erase) clearBlock(cx, cy);
                    else drawBlock(cx, cy, color);
                }
            }
        }
    }
}

static void lockPiece() {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (PIECES[currentPiece][currentRot][r] & (1 << (3 - c))) {
                int cx = px + c;
                int cy = py + r;
                if (cy >= 0 && cy < ROWS && cx >= 0 && cx < COLS) {
                    grid[cy][cx] = currentPiece + 1;
                }
            }
        }
    }
}

static void clearLines() {
    int cleared = 0;
    for (int r = ROWS - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c] == 0) { full = false; break; }
        }
        if (full) {
            // Shift everything down
            for (int r2 = r; r2 > 0; r2--) {
                for (int c = 0; c < COLS; c++) {
                    grid[r2][c] = grid[r2 - 1][c];
                }
            }
            for (int c = 0; c < COLS; c++) grid[0][c] = 0;
            cleared++;
            r++; // re-check same row
        }
    }
    if (cleared > 0) {
        lines += cleared;
        score += cleared * 100 * level;
        level = lines / 10 + 1;
        fallDelay = max(50, 500 - (level - 1) * 30);
        Sounds::sfxVictory();
        // Redraw grid
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (grid[r][c]) drawBlock(c, r, PIECE_COLORS[grid[r][c] - 1]);
                else clearBlock(c, r);
            }
        }
        // Update HUD
        tft.fillRect(SIDE_X, GRID_Y, SCREEN_W - SIDE_X - 4, 80, TFT_BLACK);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(SIDE_X, GRID_Y + 4);
        tft.print("Score");
        tft.setCursor(SIDE_X, GRID_Y + 20);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print(score);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(SIDE_X, GRID_Y + 42);
        tft.print("Level");
        tft.setCursor(SIDE_X, GRID_Y + 58);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.print(level);
    }
}

static void spawnNewPiece() {
    currentPiece = nextPiece;
    currentRot = 0;
    px = COLS / 2 - 2;
    py = -1;
    nextPiece = random(0, 7);

    if (!pieceFits(currentPiece, currentRot, px, py + 1)) {
        isGameOver = true;
        g_app.highScores[5] = max(g_app.highScores[5], (uint32_t)score);
        NVS::save();
        return;
    }

    // Show next piece in side panel
    tft.fillRect(SIDE_X, GRID_Y + 90, SCREEN_W - SIDE_X - 4, 70, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(SIDE_X, GRID_Y + 90);
    tft.print("Next");
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int x = SIDE_X + c * 12;
            int y = GRID_Y + 110 + r * 12;
            if (PIECES[nextPiece][0][r] & (1 << (3 - c))) {
                tft.fillRect(x, y, 10, 10, PIECE_COLORS[nextPiece]);
            } else {
                tft.fillRect(x, y, 10, 10, TFT_BLACK);
            }
        }
    }
}

static void drawGrid() {
    // Clear game area
    tft.fillRect(GRID_X - 2, GRID_Y - 2, GRID_W + 4, GRID_H + 4, TFT_NAVY);
    tft.drawRect(GRID_X - 2, GRID_Y - 2, GRID_W + 4, GRID_H + 4, TFT_WHITE);
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            clearBlock(c, r);
}

// ── Overlay ────────────────────────────────────
static void drawOverlay() {
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, TFT_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, TFT_RED);

    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(120, 70);
    tft.print("GAME OVER");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(140, 115);
    tft.print("Score: ");
    tft.print(score);
    tft.setCursor(140, 140);
    tft.print("Lines: ");
    tft.print(lines);

    int y1 = 190, y2 = 230;
    uint16_t colA = (gameOverChoice == 0) ? TFT_CYAN : TFT_DARKGREY;
    uint16_t colB = (gameOverChoice == 1) ? TFT_CYAN : TFT_DARKGREY;

    tft.setTextSize(2);
    tft.setTextColor(colA, TFT_BLACK);
    tft.setCursor(170, y1);
    tft.print("> Play Again");

    tft.setTextColor(colB, TFT_BLACK);
    tft.setCursor(170, y2);
    tft.print("  Back to Menu");

    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setCursor(110, 275);
    tft.print("LEFT/RIGHT: select  SW: confirm");
}

// ── Lock + spawn helper (fixes "half disappear" bug) ──
static void lockAndSpawn() {
    lockPiece();
    clearLines();
    spawnNewPiece();
    if (isGameOver) {
        Sounds::sfxGameOver();
    } else {
        // Full grid redraw ensures the locked piece stays visible
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (grid[r][c]) drawBlock(c, r, PIECE_COLORS[grid[r][c] - 1]);
                else clearBlock(c, r);
            }
        }
        // Draw the new piece at its starting position
        drawPiece(currentPiece, currentRot, px, py, false);
    }
}

// ── Main run ───────────────────────────────────
void run() {
    gameStarted = false;
    isGameOver = false;
    score = 0;
    level = 1;
    lines = 0;
    fallDelay = 500;
    gameOverChoice = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            grid[r][c] = 0;

    tft.fillScreen(TFT_BLACK);

    // Title screen
    Display::drawPanel(60, 70, SCREEN_W - 120, 180, TFT_NAVY, TFT_CYAN, 12);
    Display::drawCentredText("TETRIS", 90, 3, TFT_CYAN);
    Display::drawCentredText("Clear lines!", 135, 2, TFT_WHITE);
    Display::drawCentredText("LEFT/RIGHT: move", 168, 1, TFT_LIGHTGREY);
    Display::drawCentredText("TOP: rotate  SW: drop", 184, 1, TFT_LIGHTGREY);
    Display::drawCentredText("Press SW to start", 215, 1, TFT_LIGHTGREY);
    Sounds::sfxSelect();

    while (true) {
        Input::update();
        if (Input::pressed(Input::SW) || Input::pressed(Input::RIGHT)) {
            Input::beep(25); delay(150);
            gameStarted = true;
            break;
        }
        delay(16);
    }

    // Init game
    drawGrid();
    nextPiece = random(0, 7);
    spawnNewPiece();

    // Draw side info
    tft.fillRect(SIDE_X, GRID_Y, SCREEN_W - SIDE_X - 4, 80, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(SIDE_X, GRID_Y + 4);
    tft.print("Score");
    tft.setCursor(SIDE_X, GRID_Y + 20);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print(score);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(SIDE_X, GRID_Y + 42);
    tft.print("Level");
    tft.setCursor(SIDE_X, GRID_Y + 58);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.print(level);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(SIDE_X, GRID_Y + 90);
    tft.print("Next");

    lastFall = millis();

    // ── Game loop ──
    while (true) {
        Input::update();

        if (!isGameOver) {
            // Exit with BOTTOM
            if (Input::pressed(Input::BOTTOM)) {
                Sounds::sfxBack();
                return;
            }

            // ── Input ──
            int oldPx = px;
            int oldPy = py;
            int oldRot = currentRot;

            if (Input::pressed(Input::LEFT)) {
                if (pieceFits(currentPiece, currentRot, px - 1, py))
                    px--;
            }
            if (Input::pressed(Input::RIGHT)) {
                if (pieceFits(currentPiece, currentRot, px + 1, py))
                    px++;
            }
            if (Input::pressed(Input::TOP)) {
                int newRot = (currentRot + 1) % 4;
                if (pieceFits(currentPiece, newRot, px, py))
                    currentRot = newRot;
                else {
                    // Wall kick: try shifting left or right
                    if (pieceFits(currentPiece, newRot, px - 1, py))
                        { currentRot = newRot; px--; }
                    else if (pieceFits(currentPiece, newRot, px + 1, py))
                        { currentRot = newRot; px++; }
                }
            }

            // Hard drop with SW
            if (Input::pressed(Input::SW)) {
                while (pieceFits(currentPiece, currentRot, px, py + 1))
                    py++;
                lockAndSpawn();
                lastFall = millis();
            } else {
                // Soft drop with BOTTOM held
                if (Input::held(Input::BOTTOM)) {
                    if (millis() - lastFall > 50) {
                        if (pieceFits(currentPiece, currentRot, px, py + 1)) {
                            drawPiece(currentPiece, currentRot, px, py, true);
                            py++;
                            drawPiece(currentPiece, currentRot, px, py, false);
                        } else {
                            lockAndSpawn();
                            oldPx = px; oldPy = py; oldRot = currentRot;
                        }
                        lastFall = millis();
                    }
                } else {
                    // Auto fall
                    if (millis() - lastFall > (unsigned long)fallDelay) {
                        if (pieceFits(currentPiece, currentRot, px, py + 1)) {
                            drawPiece(currentPiece, currentRot, px, py, true);
                            py++;
                            drawPiece(currentPiece, currentRot, px, py, false);
                        } else {
                            lockAndSpawn();
                            oldPx = px; oldPy = py; oldRot = currentRot;
                        }
                        lastFall = millis();
                    }
                }

                // Redraw piece if position changed
                if (px != oldPx || py != oldPy || currentRot != oldRot) {
                    drawPiece(currentPiece, oldRot, oldPx, oldPy, true);
                    drawPiece(currentPiece, currentRot, px, py, false);
                }
            }

        } else {
            // ── Game Over Overlay ──
            drawOverlay();

            bool chosen = false;
            while (!chosen) {
                Input::update();

                if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                    gameOverChoice = 1 - gameOverChoice;
                    Input::beep(25);
                    drawOverlay();
                }

                if (Input::pressed(Input::SW)) {
                    Input::beep(25); delay(150);
                    chosen = true;
                }

                if (Input::pressed(Input::BOTTOM)) {
                    delay(150);
                    return;
                }

                delay(16);
            }

            if (gameOverChoice == 0) {
                // Reset game
                isGameOver = false;
                score = 0;
                level = 1;
                lines = 0;
                fallDelay = 500;
                gameOverChoice = 0;
                for (int r = 0; r < ROWS; r++)
                    for (int c = 0; c < COLS; c++)
                        grid[r][c] = 0;
                drawGrid();
                nextPiece = random(0, 7);
                spawnNewPiece();
                // Re-draw side info
                tft.fillRect(SIDE_X, GRID_Y, SCREEN_W - SIDE_X - 4, 80, TFT_BLACK);
                tft.setTextSize(1);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setCursor(SIDE_X, GRID_Y + 4);
                tft.print("Score");
                tft.setCursor(SIDE_X, GRID_Y + 20);
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.print(score);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setCursor(SIDE_X, GRID_Y + 42);
                tft.print("Level");
                tft.setCursor(SIDE_X, GRID_Y + 58);
                tft.setTextColor(TFT_CYAN, TFT_BLACK);
                tft.print(level);
                lastFall = millis();
            } else {
                return;
            }
        }

        delay(8);
    }
}

} // namespace Tetris

#endif
