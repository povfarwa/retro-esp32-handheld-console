#ifndef DINO_RUN_H
#define DINO_RUN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "globals.h"
#include "sounds.h"

extern TFT_eSPI tft;

namespace DinoRun {
    struct Obstacle { int x; bool active; };

    static const int DINO_X = 60;
    static int dinoY = 210;
    static float velocity = 0;
    static const float GRAVITY = 0.8;
    static const float JUMP_FORCE = -10;
    static bool isJumping = false;
    static int score = 0;
    static int highScore = 0;
    static bool isGameOver = false;
    static bool gameStarted = false;
    static std::vector<Obstacle> obstacles;
    static unsigned long lastSpawn = 0;
    static int groundY = 230;
    static int speed = 6;
    static unsigned long lastScoreTime = 0;
    static int frameCount = 0;

    static void reset() {
        tft.fillScreen(C_WHITE);
        tft.drawLine(0, groundY, 480, groundY, C_BLACK);
        dinoY = groundY - 25;
        velocity = 0;
        isJumping = false;
        score = 0;
        isGameOver = false;
        gameStarted = true;
        speed = 6;
        obstacles.clear();
        lastSpawn = millis();
        lastScoreTime = millis();
        tft.setTextColor(C_BLACK, C_WHITE);
        tft.setTextSize(1);
        tft.drawString("HI: " + String(highScore), 380, 10, 2);
        tft.drawString("Score: 0", 10, 10, 2);
    }

    static void play() {
        if (!gameStarted) { reset(); return; }
        frameCount++;

        // --- Input ---
        if ((g_input.joyY < -40 || g_input.btnAP || g_input.joyBtnP) && !isJumping) {
            velocity = JUMP_FORCE;
            isJumping = true;
            if (g_app.soundOn) Sounds::sfxJump();
        }

        // --- Physics ---
        // Erase old dino
        tft.fillRect(DINO_X - 2, (int)(dinoY - velocity - 2) - 2, 24, 30, C_WHITE);

        velocity += GRAVITY;
        dinoY += (int)velocity;

        if (dinoY >= groundY - 25) {
            dinoY = groundY - 25;
            velocity = 0;
            isJumping = false;
        }

        // --- Spawn obstacles ---
        if (millis() - lastSpawn > (unsigned long)random(800, 2000)) {
            obstacles.push_back({480, true});
            lastSpawn = millis();
        }

        // --- Move obstacles & check collision ---
        for (auto& o : obstacles) {
            if (!o.active) continue;
            // Erase
            tft.fillRect(o.x, groundY - 30, 18, 30, C_WHITE);
            o.x -= speed;
            if (o.x < -20) { o.active = false; continue; }

            // Draw cactus
            tft.fillRect(o.x, groundY - 30, 12, 30, C_GREEN);
            tft.fillRect(o.x + 3, groundY - 36, 6, 10, C_GREEN);
            tft.fillRect(o.x - 4, groundY - 22, 6, 12, C_GREEN);
            tft.fillRect(o.x + 10, groundY - 18, 6, 8, C_GREEN);

            // Collision check
            if (o.x < DINO_X + 18 && o.x + 12 > DINO_X && dinoY + 25 > groundY - 30) {
                isGameOver = true;
                if (g_app.soundOn) Sounds::sfxGameOver();
                highScore = max(highScore, score);
                g_app.highScores[4] = max(g_app.highScores[4], (uint32_t)score);
                return;
            }
        }

        // --- Draw dino ---
        // Body
        tft.fillRect(DINO_X, dinoY, 18, 25, 0xBE40); // brownish
        // Head
        tft.fillRect(DINO_X + 14, dinoY - 8, 14, 14, 0xBE40);
        // Eye
        tft.fillCircle(DINO_X + 22, dinoY - 4, 3, C_BLACK);
        // Legs (animated)
        if ((frameCount / 8) % 2 == 0) {
            tft.fillRect(DINO_X + 2, dinoY + 22, 5, 5, 0xBE40);
            tft.fillRect(DINO_X + 11, dinoY + 22, 5, 5, 0xBE40);
        } else {
            tft.fillRect(DINO_X + 2, dinoY + 25, 5, 5, 0xBE40);
            tft.fillRect(DINO_X + 11, dinoY + 25, 5, 5, 0xBE40);
        }
        // Tail
        tft.fillRect(DINO_X - 6, dinoY + 4, 6, 6, 0xBE40);

        // --- Score ---
        if (millis() - lastScoreTime > 300) {
            score++;
            lastScoreTime = millis();
            tft.fillRect(0, 0, 100, 20, C_WHITE);
            tft.setTextColor(C_BLACK, C_WHITE);
            tft.drawString("Score: " + String(score), 10, 10, 2);

            // Gradual speed increase
            if (score % 50 == 0) speed = min(14, speed + 1);
        }

        // Cleanup
        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), [](Obstacle& o) { return !o.active; }), obstacles.end());

        delay(20);
    }

    static void gameOverLoop() {
        tft.fillScreen(C_BLACK);
        tft.setTextColor(C_RED, C_BLACK);
        tft.setTextSize(3);
        tft.drawCentreString("GAME OVER", 240, 100, 1);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(2);
        tft.drawCentreString("Score: " + String(score), 240, 150, 1);
        tft.drawCentreString("Best: " + String(highScore), 240, 180, 1);
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
