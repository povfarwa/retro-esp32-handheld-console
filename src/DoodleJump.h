#ifndef DOODLE_JUMP_H
#define DOODLE_JUMP_H

#include <Arduino.h>
#include <algorithm>
#include "config.h"
#include "globals.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "nvs_save.h"

namespace DoodleJump {

// ── Constants ──
static const int PLAYER_W = 18;
static const int PLAYER_H = 22;
static const int PLAT_W   = 44;
static const int PLAT_H   = 8;
static const int MAX_PLATFORMS = 30;

static const float GRAVITY      = 0.6;
static const float JUMP_VEL     = -9.5;
static const int   MOVE_SPEED   = 4;
static const int   SCROLL_MARGIN = 100;  // how far from center before camera scrolls

// ── Colors ──
static const uint16_t C_DOODLER = 0xFD20;   // orange
static const uint16_t C_DOODLER2= 0xFBE0;   // lighter orange for face
static const uint16_t C_PLATFORM= 0x07E0;   // green
static const uint16_t C_PLAT_TOP= 0x03E0;   // darker green edge

// ── State ──
static float playerX = 240;
static float playerY = 280;
static float playerVY = 0;
static int   playerDir = 1;  // 1=right, -1=left
static int   score = 0;
static int   highScore = 0;
static bool  isGameOver = false;
static bool  gameStarted = false;
static float cameraY = 0;
static int   gameOverChoice = 0;

struct Platform {
    int x, y, w;
    bool active;
};
static Platform platforms[MAX_PLATFORMS];
static int platformCount = 0;

// ── Helpers ──
static void generateInitialPlatforms() {
    platformCount = 0;
    // Start platform directly under player
    platforms[platformCount++] = {200, 300, PLAT_W, true};
    // Generate upward
    for (int i = 1; i < 15; i++) {
        int px = random(20, SCREEN_W - PLAT_W - 20);
        int py = 300 - i * 40;
        platforms[platformCount++] = {px, py, PLAT_W, true};
    }
}

static void generateNewPlatformAbove() {
    int px = random(20, SCREEN_W - PLAT_W - 20);
    int py = platforms[platformCount - 1].y - 40;
    if (platformCount < MAX_PLATFORMS) {
        platforms[platformCount++] = {px, py, PLAT_W, true};
    } else {
        // Replace lowest inactive platform
        int lowestIdx = -1;
        int lowestY = -99999;
        for (int i = 0; i < MAX_PLATFORMS; i++) {
            if (!platforms[i].active) continue;
            int screenY = platforms[i].y - (int)cameraY;
            if (screenY > SCREEN_H + 50) {
                if (platforms[i].y > lowestY) {
                    lowestY = platforms[i].y;
                    lowestIdx = i;
                }
            }
        }
        if (lowestIdx >= 0) {
            platforms[lowestIdx] = {px, py, PLAT_W, true};
        }
    }
}

static void reset() {
    tft.fillScreen(C_BLACK);
    playerX = 240;
    playerY = 280;
    playerVY = 0;
    playerDir = 1;
    score = 0;
    isGameOver = false;
    cameraY = 0;
    gameOverChoice = 0;

    generateInitialPlatforms();
}

// ── Drawing ──
static void drawPlayer(int sx, int sy) {
    // Body (rectangle)
    tft.fillRect(sx, sy, PLAYER_W, PLAYER_H, C_DOODLER);
    // Head (circle on top)
    tft.fillCircle(sx + PLAYER_W / 2, sy, 10, C_DOODLER);
    // Eyes
    if (playerDir > 0) {
        tft.fillCircle(sx + PLAYER_W / 2 + 3, sy - 2, 2, TFT_WHITE);
        tft.fillCircle(sx + PLAYER_W / 2 + 3, sy - 2, 1, TFT_BLACK);
    } else {
        tft.fillCircle(sx + PLAYER_W / 2 - 3, sy - 2, 2, TFT_WHITE);
        tft.fillCircle(sx + PLAYER_W / 2 - 3, sy - 2, 1, TFT_BLACK);
    }
    // Hat / cap
    tft.fillRect(sx + 3, sy - 12, PLAYER_W - 6, 4, 0x001F); // blue cap
    tft.fillCircle(sx + PLAYER_W / 2, sy - 12, 6, 0x001F);
}

static void erasePlayer(int sx, int sy) {
    tft.fillRect(sx, sy - 14, PLAYER_W, PLAYER_H + 14, C_BLACK);
}

static void drawPlatform(int px, int py, int pw) {
    tft.fillRoundRect(px, py, pw, PLAT_H, 3, C_PLATFORM);
    tft.fillRoundRect(px, py, pw, 3, 2, C_PLAT_TOP);
}

static void erasePlatform(int px, int py, int pw) {
    tft.fillRect(px, py, pw, PLAT_H, C_BLACK);
}

static void drawHUD() {
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_BLACK);
    tft.setCursor(10, 10);
    tft.print("SCORE: ");
    tft.print(score);
    tft.setCursor(SCREEN_W - 80, 10);
    tft.print("BEST: ");
    tft.print(highScore);
}

// ── Game Over ──
static void drawOverlay() {
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, C_DOODLER);

    tft.setTextSize(3);
    tft.setTextColor(C_DOODLER, C_BLACK);
    tft.setCursor(100, 60);
    tft.print("GAME OVER");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, C_BLACK);
    tft.setCursor(150, 110);
    tft.print("Score: ");
    tft.print(score);
    tft.setCursor(150, 135);
    tft.print("Best: ");
    tft.print(highScore);

    int y1 = 185, y2 = 225;
    uint16_t colA = (gameOverChoice == 0) ? C_PLATFORM : TFT_DARKGREY;
    uint16_t colB = (gameOverChoice == 1) ? C_PLATFORM : TFT_DARKGREY;

    tft.setTextSize(2);
    tft.setTextColor(colA, C_BLACK);
    tft.setCursor(170, y1);
    tft.print("> Play Again");

    tft.setTextColor(colB, C_BLACK);
    tft.setCursor(170, y2);
    tft.print("  Back to Menu");

    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, C_BLACK);
    tft.setCursor(110, 275);
    tft.print("LEFT/RIGHT: select  SW: confirm");
}

// ── Game loop body ──
static void play() {
    if (!gameStarted) { reset(); return; }

    Input::Axis ax = Input::axis();

    // ── Input ──
    if (ax.x > 30 || Input::pressed(Input::RIGHT)) {
        playerDir = 1;
        playerX += MOVE_SPEED;
    } else if (ax.x < -30 || Input::pressed(Input::LEFT)) {
        playerDir = -1;
        playerX -= MOVE_SPEED;
    }
    // Wrap around screen
    if (playerX < -PLAYER_W) playerX = SCREEN_W;
    if (playerX > SCREEN_W) playerX = -PLAYER_W;

    // ── Physics ──
    playerVY += GRAVITY;
    playerY += playerVY;

    // ── Platform collision (only when falling) ──
    if (playerVY >= 0) {
        for (int i = 0; i < platformCount; i++) {
            if (!platforms[i].active) continue;
            Platform& p = platforms[i];
            int sx = (int)playerX;
            int sy = (int)playerY;
            // Check if doodler's feet land on platform
            if (sx + 4 < p.x + p.w && sx + PLAYER_W - 4 > p.x &&
                sy + PLAYER_H >= p.y && sy + PLAYER_H <= p.y + PLAT_H + 6) {
                playerVY = JUMP_VEL;
                playerY = p.y - PLAYER_H;
                Sounds::sfxClick();
                // Score for landing on new platform
                int platScreenY = p.y - (int)cameraY;
                if (platScreenY < SCREEN_H - 80) {
                    score++;
                }
            }
        }
    }

    // ── Camera scroll ──
    float targetCamera = playerY - (SCREEN_H / 2);
    if (targetCamera < cameraY) {
        cameraY += (cameraY - targetCamera) * 0.1;
        if (cameraY - targetCamera < 1) cameraY = targetCamera;
    }

    // ── Generate new platforms ahead ──
    int highestY = 99999;
    for (int i = 0; i < platformCount; i++) {
        if (platforms[i].active && platforms[i].y < highestY) {
            highestY = platforms[i].y;
        }
    }
    if (highestY > (int)cameraY - 100) {
        generateNewPlatformAbove();
    }

    // ── Fall death ──
    int screenPlayerY = (int)playerY - (int)cameraY;
    if (screenPlayerY > SCREEN_H + 50) {
        isGameOver = true;
        highScore = max(highScore, score);
        g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
        NVS::save();
        Sounds::sfxGameOver();
        return;
    }

    // ── Drawing ──
    // Clear screen
    tft.fillScreen(C_BLACK);

    // Draw platforms (only visible ones)
    for (int i = 0; i < platformCount; i++) {
        if (!platforms[i].active) continue;
        int sy = platforms[i].y - (int)cameraY;
        if (sy > -PLAT_H && sy < SCREEN_H + 10) {
            drawPlatform(platforms[i].x, sy, platforms[i].w);
        }
        // Remove platforms far below screen
        if (sy > SCREEN_H + 100) {
            platforms[i].active = false;
        }
    }

    // Draw player
    int sPlayerY = (int)playerY - (int)cameraY;
    drawPlayer((int)playerX, sPlayerY);

    // HUD
    drawHUD();

    // Score update sound every 50 points
    if (score > 0 && score % 50 == 0) {
        // One-time sound handled by scoring in platform landing
    }

    delay(16);
}

// ── Main run ──
void run() {
    gameStarted = false;
    isGameOver = false;
    gameOverChoice = 0;
    score = 0;
    playerX = 240;
    playerY = 280;
    playerVY = 0;
    cameraY = 0;

    tft.fillScreen(C_BLACK);

    // Title screen
    Display::drawPanel(40, 50, SCREEN_W - 80, 220, 0x001F, C_PLATFORM, 12);

    // Draw a little doodler on title
    drawPlayer(230, 110);

    Display::drawCentredText("DOODLE JUMP", 80, 3, C_PLATFORM);
    Display::drawCentredText("Jump & climb!", 155, 2, TFT_WHITE);
    Display::drawCentredText("LEFT/RIGHT: move  SW: start", 195, 1, TFT_LIGHTGREY);
    Display::drawCentredText("Press SW to start", 230, 1, TFT_LIGHTGREY);
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

    // ── Main game loop ──
    while (true) {
        Input::update();

        if (!isGameOver) {
            // Exit with BOTTOM button
            if (Input::pressed(Input::BOTTOM)) {
                Sounds::sfxBack();
                return;
            }
            play();
        } else {
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

                if (Input::pressed(Input::TOP)) {
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
        }

        delay(16);
    }
}

} // namespace DoodleJump

#endif
