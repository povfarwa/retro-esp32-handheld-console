#ifndef RACING_CAR_H
#define RACING_CAR_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "globals.h"
#include "sounds.h"
#include "nvs_save.h"

extern TFT_eSPI tft;

namespace RacingCar {
    struct EnemyCar {
        int x, y;
        uint16_t color;
        bool active;
    };

    static int carX = 220;
    static const int CAR_Y = 245;
    static const int CAR_W = 32;
    static const int CAR_H = 48;
    static const int ROAD_LEFT = 40;
    static const int ROAD_RIGHT = 440;
    static const int LANE_W = (ROAD_RIGHT - ROAD_LEFT) / 3;

    static int score = 0;
    static int lives = 3;
    static bool isGameOver = false;
    static bool gameStarted = false;
    static std::vector<EnemyCar> enemies;
    static unsigned long lastEnemy = 0;
    static int enemySpeed = 4;
    static unsigned long lastScoreTime = 0;

    static void drawRoad() {
        // Road background
        tft.fillRect(ROAD_LEFT, 0, ROAD_RIGHT - ROAD_LEFT, 320, 0x3186); // dark grey asphalt
        // Lane markings (dashed)
        for (int y = 0; y < 320; y += 30) {
            tft.fillRect(ROAD_LEFT + LANE_W - 2, y, 4, 15, C_WHITE);
            tft.fillRect(ROAD_LEFT + LANE_W * 2 - 2, y, 4, 15, C_WHITE);
        }
        // Road edges
        tft.drawLine(ROAD_LEFT, 0, ROAD_LEFT, 320, C_WHITE);
        tft.drawLine(ROAD_RIGHT, 0, ROAD_RIGHT, 320, C_WHITE);
        // Start/Finish line
        tft.fillRect(ROAD_LEFT, 0, ROAD_RIGHT - ROAD_LEFT, 6, C_WHITE);
    }

    static void reset() {
        tft.fillScreen(C_BLACK);
        carX = 220;
        score = 0;
        lives = 3;
        isGameOver = false;
        gameStarted = true;
        enemySpeed = 4;
        enemies.clear();
        drawRoad();
        tft.setTextColor(C_YELLOW, C_BLACK);
        tft.setTextSize(1);
        tft.drawString("SCORE: 0", 5, 10, 2);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.drawString("LIVES: " + String(lives), 370, 10, 2);

        // Draw player car immediately so it's visible from frame 1
        tft.fillRect(carX, CAR_Y, CAR_W, CAR_H, C_BLUE);
        tft.fillRect(carX + 4, CAR_Y + 10, 8, 12, C_CYAN);
        tft.fillRect(carX + 20, CAR_Y + 10, 8, 12, C_CYAN);
        tft.fillRect(carX + 2, CAR_Y + 4, CAR_W - 4, 6, 0x5D5D);
        tft.fillRect(carX - 3, CAR_Y + 10, 4, 10, C_BLACK);
        tft.fillRect(carX - 3, CAR_Y + CAR_H - 20, 4, 10, C_BLACK);
        tft.fillRect(carX + CAR_W - 1, CAR_Y + 10, 4, 10, C_BLACK);
        tft.fillRect(carX + CAR_W - 1, CAR_Y + CAR_H - 20, 4, 10, C_BLACK);
        tft.fillCircle(carX + 6, CAR_Y + CAR_H - 2, 3, C_YELLOW);
        tft.fillCircle(carX + CAR_W - 6, CAR_Y + CAR_H - 2, 3, C_YELLOW);
    }

    static void play() {
        if (!gameStarted) { reset(); return; }

        // --- Input ---
        int oldCarX = carX;
        if (g_input.joyX > 40 || g_input.btnAP) carX += 5;
        else if (g_input.joyX < -40 || g_input.btnDP) carX -= 5;
        carX = constrain(carX, ROAD_LEFT + 5, ROAD_RIGHT - CAR_W - 5);

        // Erase car at old position (wider to cover wheels that extend 3px beyond body)
        tft.fillRect(oldCarX - 4, CAR_Y, CAR_W + 8, CAR_H, 0x3186); // road color

        // --- Spawn enemies ---
        if (millis() - lastEnemy > (unsigned long)random(600, 1500)) {
            int lane = random(0, 3);
            int ex = ROAD_LEFT + 10 + lane * LANE_W;
            enemies.push_back({ex, -CAR_H, C_RED, true});
            lastEnemy = millis();
        }

        // --- Move enemies & check ---
        for (auto& e : enemies) {
            if (!e.active) continue;
            // Erase old (wider to cover wheels that extend beyond car body)
            tft.fillRect(e.x - 4, e.y, CAR_W + 8, CAR_H, 0x3186);
            e.y += enemySpeed;

            // Remove if off-screen
            if (e.y > 320) {
                e.active = false;
                continue;
            }

            // Draw enemy car
            tft.fillRect(e.x, e.y, CAR_W, CAR_H, e.color);
            // Windows
            tft.fillRect(e.x + 4, e.y + 8, 8, 10, C_CYAN);
            tft.fillRect(e.x + 20, e.y + 8, 8, 10, C_CYAN);
            // Wheels
            tft.fillRect(e.x - 3, e.y + 8, 4, 8, C_BLACK);
            tft.fillRect(e.x - 3, e.y + CAR_H - 16, 4, 8, C_BLACK);
            tft.fillRect(e.x + CAR_W - 1, e.y + 8, 4, 8, C_BLACK);
            tft.fillRect(e.x + CAR_W - 1, e.y + CAR_H - 16, 4, 8, C_BLACK);

            // Collision detection
            if (e.y + CAR_H > CAR_Y && e.y < CAR_Y + CAR_H &&
                e.x + CAR_W > carX && e.x < carX + CAR_W) {
                e.active = false;
                lives--;
                if (g_app.soundOn) Sounds::sfxHit();
                // Flash screen red
                tft.fillRect(carX, CAR_Y, CAR_W, CAR_H, C_RED);
                delay(100);
                tft.fillRect(carX, CAR_Y, CAR_W, CAR_H, 0x3186);
                tft.fillRect(370, 10, 80, 15, C_BLACK);
                tft.setTextColor(C_WHITE, C_BLACK);
                tft.drawString("LIVES: " + String(lives), 370, 10, 2);
                if (lives <= 0) {
                    isGameOver = true;
                    if (g_app.soundOn) Sounds::sfxGameOver();
                    g_app.highScores[5] = max(g_app.highScores[5], (uint32_t)score);
                    NVS::save();
                    return;
                }
            }
        }

        // --- Draw player car ---
        tft.fillRect(carX, CAR_Y, CAR_W, CAR_H, C_BLUE);
        // Windows
        tft.fillRect(carX + 4, CAR_Y + 10, 8, 12, C_CYAN);
        tft.fillRect(carX + 20, CAR_Y + 10, 8, 12, C_CYAN);
        // Windshield
        tft.fillRect(carX + 2, CAR_Y + 4, CAR_W - 4, 6, 0x5D5D);
        // Wheels
        tft.fillRect(carX - 3, CAR_Y + 10, 4, 10, C_BLACK);
        tft.fillRect(carX - 3, CAR_Y + CAR_H - 20, 4, 10, C_BLACK);
        tft.fillRect(carX + CAR_W - 1, CAR_Y + 10, 4, 10, C_BLACK);
        tft.fillRect(carX + CAR_W - 1, CAR_Y + CAR_H - 20, 4, 10, C_BLACK);
        // Headlights
        tft.fillCircle(carX + 6, CAR_Y + CAR_H - 2, 3, C_YELLOW);
        tft.fillCircle(carX + CAR_W - 6, CAR_Y + CAR_H - 2, 3, C_YELLOW);

        // --- Score (distance-based) ---
        if (millis() - lastScoreTime > 200) {
            score += 10;
            lastScoreTime = millis();
            tft.fillRect(0, 0, 120, 20, C_BLACK);
            tft.setTextColor(C_YELLOW, C_BLACK);
            tft.drawString("SCORE: " + String(score), 5, 10, 2);

            // Difficulty increase
            if (score % 200 == 0) {
                enemySpeed = min(12, enemySpeed + 1);
            }
        }

        // Cleanup
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](EnemyCar& e) { return !e.active; }), enemies.end());

        delay(20);
    }

    static void gameOverLoop() {
        tft.fillScreen(C_BLACK);
        tft.setTextColor(C_RED, C_BLACK);
        tft.setTextSize(3);
        tft.drawCentreString("CRASH!", 240, 100, 1);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(2);
        tft.drawCentreString("Score: " + String(score), 240, 150, 1);
        tft.drawCentreString("Press A to Restart", 240, 210, 1);
        if (g_input.btnAP || g_input.joyBtnP) {
            gameStarted = false;
            isGameOver = false;
        }
    }

    static void update() {
        if (g_input.btnCP && isGameOver) {
            gameStarted = false;
            isGameOver = false;
            return;
        }
        if (isGameOver) { gameOverLoop(); return; }
        play();
    }
}

#endif
