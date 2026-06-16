#ifndef PACMAN_GAME_H
#define PACMAN_GAME_H

#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "globals.h"
#include "nvs_save.h"

namespace PacMan {

static const int TILE       = 16;
static const int MAZE_COLS  = 21;
static const int MAZE_ROWS  = 21;
static const int MAZE_X     = (SCREEN_W - MAZE_COLS * TILE) / 2;
static const int MAZE_Y     = 10;

static const uint8_t MAZE[MAZE_ROWS][MAZE_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,2,1},
    {1,3,1,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,3,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,2,1},
    {1,2,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,2,1},
    {1,1,1,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,1,1},
    {0,0,0,0,1,2,1,2,2,2,2,2,2,2,1,2,1,0,0,0,0},
    {1,1,1,1,1,2,1,2,1,4,4,4,1,2,1,2,1,1,1,1,1},
    {0,0,0,0,0,2,1,2,1,4,4,4,1,2,1,2,0,0,0,0,0},
    {1,1,1,1,1,2,1,2,1,4,4,4,1,2,1,2,1,1,1,1,1},
    {0,0,0,0,1,2,1,2,2,2,2,2,2,2,1,2,1,0,0,0,0},
    {1,1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,2,1},
    {1,3,2,2,1,2,2,2,2,2,0,2,2,2,2,2,1,2,2,3,1},
    {1,1,1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1,1,1},
    {1,2,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
};

static int pacX, pacY;
static int pacDir;
static int pacNextDir;
static int animFrame;
static int score;
static int lives;
static int dotsRemaining;
static bool isGameOver;
static bool isWon;
static bool gameStarted;
static int gameOverChoice;

static const int DX[4] = {1,0,-1,0};
static const int DY[4] = {0,1,0,-1};

struct Ghost {
    int x, y;
    int dir;
    int color;
    bool frightened;
    bool eaten;
    int scatterTimer;
    int chaseTimer;
    int homeTimer;
};
static Ghost ghosts[4];
static const int GHOST_COLORS[4] = {TFT_RED, 0xFF80, TFT_CYAN, TFT_PINK};
static const char* GHOST_NAMES[4] = {"BLINKY","PINKY","INKY","CLYDE"};

static int frightenedTimer;
static int level;
static int pacTimer;
static int ghostMoveCounter;

static void drawTile(int col, int row, uint16_t color) {
    tft.fillRect(MAZE_X + col * TILE, MAZE_Y + row * TILE, TILE, TILE, color);
}

static void drawMaze() {
    tft.fillRect(MAZE_X - 2, MAZE_Y - 2, MAZE_COLS * TILE + 4, MAZE_ROWS * TILE + 4, 0x00AA);
    dotsRemaining = 0;
    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            uint8_t cell = MAZE[r][c];
            int x = MAZE_X + c * TILE;
            int y = MAZE_Y + r * TILE;
            if (cell == 1) {
                tft.fillRect(x, y, TILE, TILE, 0x3186);
                tft.drawRect(x, y, TILE, TILE, 0x2124);
            } else if (cell == 2) {
                tft.fillRect(x, y, TILE, TILE, TFT_BLACK);
                tft.fillCircle(x + TILE/2, y + TILE/2, 2, 0xFF80);
                dotsRemaining++;
            } else if (cell == 3) {
                tft.fillRect(x, y, TILE, TILE, TFT_BLACK);
                tft.fillCircle(x + TILE/2, y + TILE/2, 5, 0xFF80);
                dotsRemaining++;
            } else if (cell == 4 || cell == 5) {
                tft.fillRect(x, y, TILE, TILE, TFT_BLACK);
            }
        }
    }
}

static void drawPacMan(int col, int row, int dir, int frame) {
    int cx = MAZE_X + col * TILE + TILE/2;
    int cy = MAZE_Y + row * TILE + TILE/2;
    int r = TILE/2 - 1;

    float a1, a2;
    int mouthSize = (frame % 4 < 2) ? 30 : 8;
    switch (dir) {
        case 0: a1 = mouthSize * 0.01745; a2 = 360 - a1; break;
        case 1: a1 = 90 + mouthSize * 0.01745; a2 = 270 - a1; break;
        case 2: a1 = 180 + mouthSize * 0.01745; a2 = 180 - a1; break;
        case 3: a1 = 270 + mouthSize * 0.01745; a2 = 90 - a1; break;
        default: a1 = 0; a2 = 360;
    }

    tft.fillCircle(cx, cy, r, TFT_BLACK);
    tft.fillSmoothCircle(cx, cy, r, TFT_YELLOW, TFT_BLACK);

    if (mouthSize < 180) {

        for (int i = -r; i <= r; i++) {
            int halfW = sqrt(r*r - i*i);
            for (int j = 0; j < halfW; j++) {
                float angle = atan2(i, j);
                float deg = angle * 57.2958;
                if (deg < 0) deg += 360;
                bool inMouth = false;
                switch (dir) {
                    case 0: inMouth = (deg > a1 && deg < a2); break;
                    case 1: inMouth = (deg > a1 || deg < a2); break;
                    case 2: inMouth = (deg > a1 || deg < a2); break;
                    case 3: inMouth = (deg > a1 || deg < a2); break;
                }
                if (dir == 0) {
                    inMouth = (deg > a1 && deg < a2);
                } else if (dir == 1) {
                    inMouth = (deg > a1 || deg < a2 - 360);
                } else if (dir == 2) {
                    inMouth = (deg > a1 || deg < a2 - 360);
                } else if (dir == 3) {
                    inMouth = (deg > a1 || deg < a2 - 360);
                }
                if (!inMouth) {
                    int px = cx + j;
                    int py = cy + i;
                    tft.drawPixel(px, py, TFT_BLACK);
                    if (j > 0) tft.drawPixel(cx - j, py, TFT_BLACK);
                }
            }
        }
    }
}

static void erasePacMan(int col, int row) {
    int x = MAZE_X + col * TILE;
    int y = MAZE_Y + row * TILE;
    tft.fillRect(x, y, TILE, TILE, TFT_BLACK);
}

static void drawGhost(const Ghost& g) {
    int cx = MAZE_X + g.x * TILE + TILE/2;
    int cy = MAZE_Y + g.y * TILE + TILE/2 + 2;
    int r = TILE/2 - 1;

    tft.fillRect(cx - r, cy - r, r*2, r*2, TFT_BLACK);

    uint16_t color;
    if (g.frightened) color = (frightenedTimer < 2000 && (frightenedTimer / 200) % 2 == 0) ? TFT_WHITE : 0x001F;
    else if (g.eaten) color = 0x0000;
    else color = g.color;

    if (g.eaten) return;

    tft.fillCircle(cx, cy - 2, r, color);

    tft.fillRect(cx - r, cy - 2, r*2, r, color);

    int waveY = cy + r - 2;
    for (int i = 0; i < r*2; i += 4) {
        int w = (i / 4) % 2 == 0 ? 2 : 4;
        tft.fillRect(cx - r + i, waveY, 4, w, color);
    }

    if (!g.frightened) {
        tft.fillCircle(cx - 3, cy - 4, 3, TFT_WHITE);
        tft.fillCircle(cx + 3, cy - 4, 3, TFT_WHITE);
        tft.fillCircle(cx - 2, cy - 4, 1, (g.dir == 0) ? 0x0000 : 0x001F);
        tft.fillCircle(cx + 4, cy - 4, 1, (g.dir == 0) ? 0x001F : 0x0000);
    } else {

        tft.drawLine(cx - 4, cy, cx - 1, cy + 2, TFT_RED);
        tft.drawLine(cx + 4, cy, cx + 1, cy + 2, TFT_RED);
        tft.drawLine(cx - 4, cy + 1, cx - 1, cy + 3, TFT_RED);
        tft.drawLine(cx + 4, cy + 1, cx + 1, cy + 3, TFT_RED);
    }
}

static void eraseGhost(const Ghost& g) {
    int x = MAZE_X + g.x * TILE;
    int y = MAZE_Y + g.y * TILE;
    tft.fillRect(x, y, TILE, TILE, TFT_BLACK);
}

static bool isWalkable(int col, int row) {
    if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS) return false;
    uint8_t cell = MAZE[row][col];
    return cell != 1 && cell != 5;
}

static void initLevel() {

    drawMaze();

    pacX = 10; pacY = 15;
    pacDir = 0; pacNextDir = 0;
    animFrame = 0;
    pacTimer = 0;
    ghostMoveCounter = 0;

    for (int i = 0; i < 4; i++) {
        ghosts[i].x = 9 + (i % 2) * 2;
        ghosts[i].y = 9 + (i / 2);
        ghosts[i].dir = 0;
        ghosts[i].color = GHOST_COLORS[i];
        ghosts[i].frightened = false;
        ghosts[i].eaten = false;
        ghosts[i].scatterTimer = 0;
        ghosts[i].chaseTimer = 0;
        ghosts[i].homeTimer = i * 500;
    }

    frightenedTimer = 0;
}

static void reset() {
    score = 0;
    lives = 3;
    level = 1;
    isGameOver = false;
    isWon = false;
    gameStarted = false;
    gameOverChoice = 0;

    tft.fillScreen(TFT_BLACK);
    initLevel();

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(MAZE_X, MAZE_Y + MAZE_ROWS * TILE + 4);
    tft.print("SCORE: 0");
    tft.setCursor(MAZE_X + 160, MAZE_Y + MAZE_ROWS * TILE + 4);
    tft.print("LV: 1");
    tft.setCursor(MAZE_X + 250, MAZE_Y + MAZE_ROWS * TILE + 4);
    tft.print("LIVES: ");
    for (int i = 0; i < lives; i++) {
        tft.fillCircle(MAZE_X + 310 + i * 14, MAZE_Y + MAZE_ROWS * TILE + 8, 4, TFT_YELLOW);
    }
}

static void moveGhost(Ghost& g) {
    if (g.homeTimer > 0) { g.homeTimer -= 16; return; }
    if (g.eaten) {

        int targetX = 10, targetY = 9;
        int bestDist = 9999, bestDir = g.dir;
        for (int d = 0; d < 4; d++) {
            int nx = g.x + DX[d];
            int ny = g.y + DY[d];
            if (!isWalkable(nx, ny)) continue;
            if ((d + 2) % 4 == g.dir) continue;
            int dist = abs(nx - targetX) + abs(ny - targetY);
            if (dist < bestDist) { bestDist = dist; bestDir = d; }
        }
        g.dir = bestDir;
        g.x += DX[g.dir];
        g.y += DY[g.dir];
        if (g.x == 10 && g.y == 9) { g.eaten = false; g.frightened = false; }
        return;
    }

    int targetX, targetY;
    if (g.frightened) {

        int dirs[4] = {0,1,2,3};
        for (int i = 0; i < 4; i++) {
            int j = random(i, 4);
            int t = dirs[i]; dirs[i] = dirs[j]; dirs[j] = t;
        }
        for (int i = 0; i < 4; i++) {
            int nx = g.x + DX[dirs[i]];
            int ny = g.y + DY[dirs[i]];
            if (isWalkable(nx, ny) && (dirs[i] + 2) % 4 != g.dir) {
                g.dir = dirs[i];
                g.x = nx;
                g.y = ny;
                return;
            }
        }

        for (int i = 0; i < 4; i++) {
            int nx = g.x + DX[i];
            int ny = g.y + DY[i];
            if (isWalkable(nx, ny)) {
                g.dir = i;
                g.x = nx;
                g.y = ny;
                return;
            }
        }
        return;
    }

    targetX = pacX;
    targetY = pacY;

    int bestDist = 9999, bestDir = g.dir;
    for (int d = 0; d < 4; d++) {
        int nx = g.x + DX[d];
        int ny = g.y + DY[d];
        if (!isWalkable(nx, ny)) continue;
        if ((d + 2) % 4 == g.dir) continue;
        int dist = abs(nx - targetX) + abs(ny - targetY);
        if (dist < bestDist) { bestDist = dist; bestDir = d; }
    }
    g.dir = bestDir;
    g.x += DX[g.dir];
    g.y += DY[g.dir];
}

static void drawOverlay() {
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, TFT_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, 0x00AA);

    tft.setTextSize(3);
    tft.setTextColor(0x00AA, TFT_BLACK);
    tft.setCursor(110, 60);
    tft.print("GAME OVER");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(140, 110);
    tft.print("Score: ");
    tft.print(score);

    int y1 = 170, y2 = 210;
    uint16_t colA = (gameOverChoice == 0) ? TFT_YELLOW : TFT_DARKGREY;
    uint16_t colB = (gameOverChoice == 1) ? TFT_YELLOW : TFT_DARKGREY;

    tft.setTextSize(2);
    tft.setTextColor(colA, TFT_BLACK);
    tft.setCursor(170, y1);
    tft.print("> Play Again");

    tft.setTextColor(colB, TFT_BLACK);
    tft.setCursor(170, y2);
    tft.print("  Back to Menu");

    tft.setTextSize(1);
    tft.setTextColor(0x00AA, TFT_BLACK);
    tft.setCursor(110, 270);
    tft.print("LEFT/RIGHT: select  SW: confirm");
}

static void drawWinOverlay() {
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, TFT_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, TFT_YELLOW);

    tft.setTextSize(3);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(140, 60);
    tft.print("YOU WIN!");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(140, 110);
    tft.print("Score: ");
    tft.print(score);

    tft.setTextSize(2);
    tft.setTextColor((gameOverChoice == 0) ? TFT_YELLOW : TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(170, 170);
    tft.print("> Next Level");

    tft.setTextColor((gameOverChoice == 1) ? TFT_YELLOW : TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(170, 210);
    tft.print("  Back to Menu");

    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setCursor(110, 270);
    tft.print("LEFT/RIGHT: select  SW: confirm");
}

void run() {
    gameStarted = false;
    isGameOver = false;
    isWon = false;
    score = 0;
    lives = 3;
    level = 1;
    gameOverChoice = 0;

    tft.fillScreen(TFT_BLACK);

    Display::drawPanel(50, 40, SCREEN_W - 100, 240, TFT_BLACK, 0x00AA, 12);

    tft.fillCircle(240, 85, 20, TFT_YELLOW);
    tft.fillTriangle(240, 85, 260, 70, 260, 100, TFT_BLACK);

    Display::drawCentredText("DOODLE JUMP", 120, 3, TFT_YELLOW);
    Display::drawCentredText("Jump & climb!", 160, 2, TFT_WHITE);
    Display::drawCentredText("LEFT/RIGHT: move  TOP: up  BOTTOM: down", 195, 1, 0x00AA);
    Display::drawCentredText("Press SW to start", 230, 1, TFT_LIGHTGREY);
    Sounds::sfxSelect();

    while (true) {
        Input::update();
        if (Input::pressed(Input::SW)) {
            Input::beep(25); delay(150);
            gameStarted = true;
            break;
        }
        delay(16);
    }

    reset();

    while (true) {
        Input::update();

        if (!isGameOver && !isWon) {

            if (Input::pressed(Input::LEFT)) pacNextDir = 2;
            else if (Input::pressed(Input::RIGHT)) pacNextDir = 0;
            else if (Input::pressed(Input::TOP)) pacNextDir = 3;
            else if (Input::pressed(Input::BOTTOM)) pacNextDir = 1;

            pacTimer++;
            if (pacTimer >= 4) {
                pacTimer = 0;

                int nx = pacX + DX[pacNextDir];
                int ny = pacY + DY[pacNextDir];
                if (isWalkable(nx, ny)) {
                    pacDir = pacNextDir;
                }

                nx = pacX + DX[pacDir];
                ny = pacY + DY[pacDir];
                if (isWalkable(nx, ny)) {
                    erasePacMan(pacX, pacY);
                    pacX = nx;
                    pacY = ny;
                    animFrame++;

                    uint8_t cell = MAZE[pacY][pacX];
                    if (cell == 2) {
                        score += 10;
                        dotsRemaining--;

                        const_cast<uint8_t&>(MAZE[pacY][pacX]) = 0;

                        tft.fillRect(MAZE_X, MAZE_Y + MAZE_ROWS * TILE + 2, 100, 10, TFT_BLACK);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.setCursor(MAZE_X, MAZE_Y + MAZE_ROWS * TILE + 4);
                        tft.print("SCORE: ");
                        tft.print(score);
                    } else if (cell == 3) {
                        score += 50;
                        dotsRemaining--;
                        const_cast<uint8_t&>(MAZE[pacY][pacX]) = 0;
                        frightenedTimer = 5000;
                        for (int i = 0; i < 4; i++) {
                            if (!ghosts[i].eaten) ghosts[i].frightened = true;
                        }
                        Sounds::sfxPowerUp();

                        tft.fillRect(MAZE_X, MAZE_Y + MAZE_ROWS * TILE + 2, 100, 10, TFT_BLACK);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.setCursor(MAZE_X, MAZE_Y + MAZE_ROWS * TILE + 4);
                        tft.print("SCORE: ");
                        tft.print(score);
                    }

                    drawPacMan(pacX, pacY, pacDir, animFrame);
                }

                ghostMoveCounter++;
                if (ghostMoveCounter >= 2) {
                    ghostMoveCounter = 0;
                    for (int i = 0; i < 4; i++) {
                        eraseGhost(ghosts[i]);
                        moveGhost(ghosts[i]);
                        drawGhost(ghosts[i]);
                    }
                }

                for (int i = 0; i < 4; i++) {
                    if (ghosts[i].x == pacX && ghosts[i].y == pacY) {
                        if (ghosts[i].frightened && !ghosts[i].eaten) {

                            ghosts[i].eaten = true;
                            ghosts[i].frightened = false;
                            ghosts[i].homeTimer = 0;
                            score += 200;
                            Sounds::sfxEnemyHit();

                            tft.fillRect(MAZE_X, MAZE_Y + MAZE_ROWS * TILE + 2, 100, 10, TFT_BLACK);
                            tft.setTextColor(TFT_WHITE, TFT_BLACK);
                            tft.setCursor(MAZE_X, MAZE_Y + MAZE_ROWS * TILE + 4);
                            tft.print("SCORE: ");
                            tft.print(score);
                        } else if (!ghosts[i].eaten && !ghosts[i].frightened) {

                            lives--;
                            Sounds::sfxPlayerHit();
                            tft.fillRect(MAZE_X + 250, MAZE_Y + MAZE_ROWS * TILE + 2, 100, 10, TFT_BLACK);
                            tft.setTextColor(TFT_WHITE, TFT_BLACK);
                            tft.setCursor(MAZE_X + 250, MAZE_Y + MAZE_ROWS * TILE + 4);
                            tft.print("LIVES: ");
                            for (int j = 0; j < lives; j++) {
                                tft.fillCircle(MAZE_X + 310 + j * 14, MAZE_Y + MAZE_ROWS * TILE + 8, 4, TFT_YELLOW);
                            }

                            if (lives <= 0) {
                                isGameOver = true;
                                g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
                                NVS::save();
                                Sounds::sfxGameOver();
                            } else {

                                pacX = 10; pacY = 15;
                                pacDir = 0; pacNextDir = 0;
                                drawPacMan(pacX, pacY, pacDir, 0);
                                for (int j = 0; j < 4; j++) {
                                    eraseGhost(ghosts[j]);
                                    ghosts[j].x = 9 + (j % 2) * 2;
                                    ghosts[j].y = 9 + (j / 2);
                                    ghosts[j].dir = 0;
                                    ghosts[j].frightened = false;
                                    ghosts[j].eaten = false;
                                    ghosts[j].homeTimer = j * 500;
                                    drawGhost(ghosts[j]);
                                }
                                delay(500);
                            }
                        }
                    }
                }

                if (dotsRemaining <= 0) {
                    isWon = true;
                    g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
                    NVS::save();
                    Sounds::sfxVictory();
                }
            }

            if (frightenedTimer > 0) {
                frightenedTimer -= 16;
                if (frightenedTimer <= 0) {
                    frightenedTimer = 0;
                    for (int i = 0; i < 4; i++) {
                        ghosts[i].frightened = false;
                    }
                }
            }

        } else if (isGameOver) {
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
                reset();
            } else {
                return;
            }
        } else if (isWon) {
            drawWinOverlay();
            bool chosen = false;
            while (!chosen) {
                Input::update();
                if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                    gameOverChoice = 1 - gameOverChoice;
                    Input::beep(25);
                    drawWinOverlay();
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
                level++;
                initLevel();
                isWon = false;
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setCursor(MAZE_X + 160, MAZE_Y + MAZE_ROWS * TILE + 4);
                tft.print("LV: ");
                tft.print(level);
            } else {
                return;
            }
        }

        delay(16);
    }
}

}

#endif
