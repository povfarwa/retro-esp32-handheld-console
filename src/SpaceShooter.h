#ifndef SPACE_SHOOTER_H
#define SPACE_SHOOTER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "globals.h"
#include "sounds.h"
#include "nvs_save.h"

extern TFT_eSPI tft;

namespace SpaceShooter {
    struct Bullet { int x, y; bool active; };
    struct Enemy { int x, y; bool active; };

    static int shipX = 240;
    static const int SHIP_Y = 280;
    static int score = 0;
    static int lives = 3;
    static bool isGameOver = false;
    static bool gameStarted = false;
    static std::vector<Bullet> bullets;
    static std::vector<Enemy> enemies;
    static unsigned long lastEnemySpawn = 0;
    static unsigned long lastFire = 0;
    static int enemySpeed = 3;
    static int spawnRate = 1200;
    static int level = 1;

    static void reset() {
        tft.fillScreen(C_BLACK);
        shipX = 240;
        score = 0;
        lives = 3;
        level = 1;
        isGameOver = false;
        gameStarted = true;
        enemySpeed = 3;
        spawnRate = 1200;
        bullets.clear();
        enemies.clear();
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(1);
        tft.drawString("SCORE: 0", 5, 5, 2);
        tft.drawString("LIVES: 3", 380, 5, 2);
        tft.drawString("LV:1", 200, 5, 2);
    }

    static void play() {
        if (!gameStarted) { reset(); return; }

        // --- Input ---
        if (g_input.joyX > 40 || g_input.btnAP) shipX += 5;
        else if (g_input.joyX < -40 || g_input.btnDP) shipX -= 5;
        shipX = constrain(shipX, 10, 440);

        // Fire
        if ((g_input.joyBtnP || g_input.btnBP || g_input.joyY < -40) && millis() - lastFire > 300) {
            bullets.push_back({shipX + 12, SHIP_Y, true});
            if (g_app.soundOn) Sounds::sfxJump();
            lastFire = millis();
        }

        // --- Clear ship area ---
        tft.fillRect(shipX - 2, SHIP_Y, 34, 22, C_BLACK);

        // --- Move & draw bullets ---
        for (auto& b : bullets) {
            if (!b.active) continue;
            tft.fillRect(b.x, b.y, 4, 8, C_BLACK);
            b.y -= 6;
            if (b.y < 0) b.active = false;
            else tft.fillRect(b.x, b.y, 4, 8, C_YELLOW);
        }

        // --- Spawn enemies ---
        if (millis() - lastEnemySpawn > (unsigned long)spawnRate) {
            enemies.push_back({(int)random(20, 460), 0, true});
            lastEnemySpawn = millis();
        }

        // --- Move & draw enemies ---
        for (auto& e : enemies) {
            if (!e.active) continue;
            tft.fillCircle(e.x, e.y, 8, C_BLACK);
            e.y += enemySpeed;
            if (e.y > 320) { e.active = false; continue; }

            // Collision with ship
            if (e.y + 8 > SHIP_Y && e.y - 8 < SHIP_Y + 20 &&
                e.x + 8 > shipX && e.x - 8 < shipX + 30) {
                e.active = false;
                lives--;
                if (g_app.soundOn) Sounds::sfxHit();
                tft.fillRect(380, 5, 80, 15, C_BLACK);
                tft.setTextColor(C_WHITE, C_BLACK);
                tft.drawString("LIVES: " + String(lives), 380, 5, 2);
                if (lives <= 0) {
                    isGameOver = true;
                    if (g_app.soundOn) Sounds::sfxGameOver();
                    g_app.highScores[1] = max(g_app.highScores[1], (uint32_t)score);
                    NVS::save();
                    return;
                }
                continue;
            }
            tft.fillCircle(e.x, e.y, 8, C_MAGENTA);
        }

        // --- Bullet-Enemy collision ---
        for (auto& b : bullets) {
            if (!b.active) continue;
            for (auto& e : enemies) {
                if (!e.active) continue;
                if (abs(b.x - e.x) < 12 && abs(b.y - e.y) < 12) {
                    b.active = false;
                    e.active = false;
                    tft.fillCircle(e.x, e.y, 10, C_BLACK);
                    score += 10 + level * 5;
                    if (g_app.soundOn) Sounds::sfxPoint();
                    tft.fillRect(0, 0, 120, 15, C_BLACK);
                    tft.setTextColor(C_WHITE, C_BLACK);
                    tft.drawString("SCORE: " + String(score), 5, 5, 2);

                    // Level up every 100 points
                    if (score > level * 100) {
                        level++;
                        enemySpeed = min(8, enemySpeed + 1);
                        spawnRate = max(400, spawnRate - 100);
                        tft.fillRect(200, 5, 60, 15, C_BLACK);
                        tft.drawString("LV:" + String(level), 200, 5, 2);
                        if (g_app.soundOn) Sounds::sfxVictory();
                    }
                    break;
                }
            }
        }

        // --- Draw ship ---
        tft.fillTriangle(shipX, SHIP_Y + 20, shipX + 15, SHIP_Y, shipX + 30, SHIP_Y + 20, C_BLUE);
        tft.fillRect(shipX + 12, SHIP_Y - 5, 6, 8, C_RED);

        // --- Cleanup ---
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& b) { return !b.active; }), bullets.end());
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](Enemy& e) { return !e.active; }), enemies.end());

        delay(20);
    }

    static void gameOverLoop() {
        tft.fillScreen(C_BLACK);
        tft.setTextColor(C_RED, C_BLACK);
        tft.setTextSize(3);
        tft.drawCentreString("GAME OVER", 240, 110, 1);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(2);
        tft.drawCentreString("Score: " + String(score), 240, 160, 1);
        tft.drawCentreString("Level: " + String(level), 240, 190, 1);
        tft.drawCentreString("Press A to Restart", 240, 230, 1);
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
