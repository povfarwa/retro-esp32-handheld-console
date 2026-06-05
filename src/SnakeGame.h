#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include "globals.h"
#include "sounds.h"
#include "nvs_save.h"

extern TFT_eSPI tft;

namespace SnakeGame {
    static const int DOT_SIZE = 12;
    static const int COLS = 480 / DOT_SIZE;
    static const int ROWS = 320 / DOT_SIZE;

    struct Point { int x; int y; };
    static std::vector<Point> snake;
    static Point food;
    static int dirX = 1, dirY = 0;
    static int nextDirX = 1, nextDirY = 0;
    static int score = 0;
    static bool isGameOver = false;
    static bool gameStarted = false;
    static unsigned long lastMove = 0;
    static int moveDelay = 100;

    static void spawnFood() {
        bool valid;
        do {
            valid = true;
            food.x = random(0, COLS) * DOT_SIZE;
            food.y = random(0, ROWS) * DOT_SIZE;
            for (auto& s : snake) {
                if (s.x == food.x && s.y == food.y) { valid = false; break; }
            }
        } while (!valid);
    }

    static void resetGame() {
        isGameOver = false;
        gameStarted = true;
        score = 0;
        dirX = 1; dirY = 0;
        nextDirX = 1; nextDirY = 0;
        moveDelay = 100;
        snake.clear();

        int cx = (COLS / 2) * DOT_SIZE;
        int cy = (ROWS / 2) * DOT_SIZE;
        snake.push_back({cx, cy});
        snake.push_back({cx - DOT_SIZE, cy});
        snake.push_back({cx - 2 * DOT_SIZE, cy});

        tft.fillScreen(C_BLACK);
        spawnFood();
        for (auto& s : snake) {
            tft.fillRect(s.x + 1, s.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, C_GREEN);
        }
        tft.fillRect(food.x + 1, food.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, C_RED);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(1);
        tft.drawString("Score: 0", 5, 5, 2);
    }

    static void play() {
        if (!gameStarted) { resetGame(); return; }

        // Input handling
        if (g_input.joyX > 40 && dirX == 0) { nextDirX = 1; nextDirY = 0; }
        else if (g_input.joyX < -40 && dirX == 0) { nextDirX = -1; nextDirY = 0; }
        else if (g_input.joyY > 40 && dirY == 0) { nextDirX = 0; nextDirY = 1; }
        else if (g_input.joyY < -40 && dirY == 0) { nextDirX = 0; nextDirY = -1; }
        else if (g_input.btnAP && dirX == 0) { nextDirX = 1; nextDirY = 0; }
        else if (g_input.btnDP && dirX == 0) { nextDirX = -1; nextDirY = 0; }
        else if (g_input.btnBP && dirY == 0) { nextDirX = 0; nextDirY = 1; }
        else if (g_input.btnCP && dirY == 0) { nextDirX = 0; nextDirY = -1; }

        unsigned long now = millis();
        if (now - lastMove < (unsigned long)moveDelay) return;
        lastMove = now;

        dirX = nextDirX; dirY = nextDirY;

        Point newHead = {snake[0].x + dirX * DOT_SIZE, snake[0].y + dirY * DOT_SIZE};

        // Wrap around edges
        if (newHead.x < 0) newHead.x = 480 - DOT_SIZE;
        else if (newHead.x >= 480) newHead.x = 0;
        if (newHead.y < 0) newHead.y = 320 - DOT_SIZE;
        else if (newHead.y >= 320) newHead.y = 0;

        // Draw new head
        snake.insert(snake.begin(), newHead);
        tft.fillRect(newHead.x + 1, newHead.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, C_GREEN);

        // Check food
        if (newHead.x == food.x && newHead.y == food.y) {
            score++;
            if (g_app.soundOn) Sounds::sfxPoint();
            moveDelay = max(40, moveDelay - 2);
            spawnFood();
            tft.fillRect(food.x + 1, food.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, C_RED);
        } else {
            // Remove tail
            Point tail = snake.back();
            tft.fillRect(tail.x, tail.y, DOT_SIZE, DOT_SIZE, C_BLACK);
            snake.pop_back();
        }

        // Self-collision check after tail removal (no false-positive
        // when head moves into the tail's just-vacated position)
        for (size_t i = 1; i < snake.size(); i++) {
            if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
                isGameOver = true;
                if (g_app.soundOn) Sounds::sfxGameOver();
                g_app.highScores[0] = max(g_app.highScores[0], (uint32_t)score);
                NVS::save();
                return;
            }
        }

        // Update score
        tft.fillRect(0, 0, 160, 20, C_BLACK);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(1);
        tft.drawString("Score: " + String(score), 5, 5, 2);
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
