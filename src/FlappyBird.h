#ifndef FLAPPY_BIRD_H
#define FLAPPY_BIRD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "globals.h"
#include "sounds.h"
#include "nvs_save.h"

extern TFT_eSPI tft;

namespace FlappyBird {
    struct Pipe {
        int x, gapY;
        bool scored;
    };

    static float birdY = 160;
    static float birdVel = 0;
    static const int BIRD_X = 80;
    static const int BIRD_SIZE = 10;
    static const float GRAVITY = 0.5;
    static const float FLAP_STRENGTH = -6.5;
    static int pipeSpeed = 3;
    static int pipeGap = 90;
    static const int PIPE_W = 36;
    static int score = 0;
    static bool isGameOver = false;
    static bool gameStarted = false;
    static std::vector<Pipe> pipes;
    static unsigned long lastPipe = 0;
    static int bgColor = 0x053F; // sky blue

    static void reset() {
        tft.fillScreen(bgColor);
        birdY = 160;
        birdVel = 0;
        score = 0;
        isGameOver = false;
        gameStarted = true;
        pipeSpeed = 3;
        pipes.clear();
        pipes.push_back({400, (int)random(50, 180), false});
        lastPipe = millis();
        tft.setTextColor(C_WHITE, bgColor);
        tft.setTextSize(1);
        tft.drawString("Score: 0", 10, 10, 2);

        // Draw ground
        tft.fillRect(0, 290, 480, 30, C_GREEN);
        tft.drawLine(0, 290, 480, 290, C_WHITE);
    }

    static void play() {
        if (!gameStarted) { reset(); return; }

        // Gravity
        birdVel += GRAVITY;
        birdY += birdVel;

        // Flap
        if (g_input.joyY < -40 || g_input.btnAP || g_input.joyBtnP) {
            birdVel = FLAP_STRENGTH;
            if (g_app.soundOn) Sounds::sfxJump();
        }

        // Clear bird trail
        tft.fillCircle(BIRD_X, (int)(birdY - birdVel - 2), BIRD_SIZE + 1, bgColor);

        // Spawn pipes
        if (millis() - lastPipe > 1600) {
            pipes.push_back({470, (int)random(40, 190), false});
            lastPipe = millis();
        }

        // Move & draw pipes
        for (auto& p : pipes) {
            // Erase old pipe
            tft.fillRect(p.x, 0, PIPE_W, p.gapY, bgColor);
            tft.fillRect(p.x, p.gapY + pipeGap, PIPE_W, 320 - p.gapY - pipeGap, bgColor);

            p.x -= pipeSpeed;

            // Draw new pipe
            tft.fillRect(p.x, 0, PIPE_W, p.gapY, C_GREEN);
            tft.fillRect(p.x, p.gapY + pipeGap, PIPE_W, 320 - p.gapY - pipeGap, C_GREEN);
            // Pipe cap
            tft.fillRect(p.x - 3, p.gapY - 10, PIPE_W + 6, 10, C_YELLOW);
            tft.fillRect(p.x - 3, p.gapY + pipeGap, PIPE_W + 6, 10, C_YELLOW);

            // Collision
            if (BIRD_X + BIRD_SIZE > p.x && BIRD_X - BIRD_SIZE < p.x + PIPE_W) {
                if (birdY - BIRD_SIZE < p.gapY || birdY + BIRD_SIZE > p.gapY + pipeGap) {
                    isGameOver = true;
                    if (g_app.soundOn) Sounds::sfxGameOver();
                    g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
                    NVS::save();
                    return;
                }
            }

            // Scoring
            if (!p.scored && p.x + PIPE_W < BIRD_X) {
                p.scored = true;
                score++;
                if (g_app.soundOn) Sounds::sfxPoint();
                tft.fillRect(0, 0, 140, 20, bgColor);
                tft.setTextColor(C_WHITE, bgColor);
                tft.drawString("Score: " + String(score), 10, 10, 2);

                // Increase difficulty
                if (score % 5 == 0) {
                    pipeSpeed = min(8, pipeSpeed + 1);
                    pipeGap = max(65, pipeGap - 3);
                }
            }
        }

        // Remove off-screen pipes
        pipes.erase(std::remove_if(pipes.begin(), pipes.end(), [](Pipe& p) { return p.x < -PIPE_W; }), pipes.end());

        // Ground / ceiling collision (bird radius = BIRD_SIZE = 10)
        // Ground at y=290 → bird center touches ground at y=280
        // Ceiling at y=0 → bird center touches ceiling at y=10
        if (birdY >= 280 || birdY < BIRD_SIZE) {
            isGameOver = true;
            if (g_app.soundOn) Sounds::sfxGameOver();
            g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
            NVS::save();
            return;
        }

        // Draw bird
        tft.fillCircle(BIRD_X, (int)birdY, BIRD_SIZE, C_YELLOW);
        tft.fillCircle(BIRD_X + 3, (int)birdY - 2, 2, C_BLACK); // eye

        delay(25);
    }

    static void gameOverLoop() {
        tft.fillScreen(C_BLACK);
        tft.setTextColor(C_RED, C_BLACK);
        tft.setTextSize(3);
        tft.drawCentreString("GAME OVER", 240, 120, 1);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(2);
        tft.drawCentreString("Score: " + String(score), 240, 170, 1);
        tft.drawCentreString("Press A to Restart", 240, 220, 1);
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
