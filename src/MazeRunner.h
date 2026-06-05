#ifndef MAZE_RUNNER_H
#define MAZE_RUNNER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "globals.h"
#include "sounds.h"

extern TFT_eSPI tft;

namespace MazeRunner {
    static const int CELL = 18;
    static const int COLS = 26;
    static const int ROWS = 17;

    static int playerX = 1, playerY = 1;
    static int goalX = 24, goalY = 15;
    static int moves = 0;
    static bool isGameOver = false;
    static bool gameStarted = false;
    static bool won = false;

    // 0=path, 1=wall
    static const byte maze[ROWS][COLS] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1},
        {1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    static void drawMaze() {
        tft.fillScreen(C_BLACK);
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (maze[r][c] == 1) {
                    tft.fillRect(c * CELL, r * CELL, CELL, CELL, 0x5AEB); // dark grey-blue walls
                }
            }
        }
        // Draw goal
        tft.fillRect(goalX * CELL, goalY * CELL, CELL, CELL, C_GREEN);
        tft.setTextColor(C_WHITE, C_GREEN);
        tft.setTextSize(1);
        tft.drawCentreString("EXIT", goalX * CELL + CELL / 2, goalY * CELL + 4, 1);

        // HUD
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.setTextSize(1);
        tft.drawString("Moves: 0", 5, 305, 2);
        tft.drawString("Find the EXIT!", 330, 305, 2);
    }

    static void reset() {
        playerX = 1; playerY = 1;
        moves = 0;
        isGameOver = false;
        won = false;
        gameStarted = true;
        drawMaze();
        tft.fillRect(playerX * CELL + 2, playerY * CELL + 2, CELL - 4, CELL - 4, C_RED);
    }

    static void play() {
        if (!gameStarted) { reset(); return; }

        int newX = playerX, newY = playerY;
        bool moved = false;

        if (g_input.btnAP || g_input.joyX > 40) { newX++; moved = true; }
        else if (g_input.btnDP || g_input.joyX < -40) { newX--; moved = true; }
        else if (g_input.btnBP || g_input.joyY > 40) { newY++; moved = true; }
        else if (g_input.btnCP || g_input.joyY < -40) { newY--; moved = true; }

        if (moved) {
            if (newX >= 0 && newX < COLS && newY >= 0 && newY < ROWS && maze[newY][newX] == 0) {
                // Erase old position
                tft.fillRect(playerX * CELL, playerY * CELL, CELL, CELL, C_BLACK);
                // Draw path line
                tft.fillRect(playerX * CELL + CELL / 2 - 1, playerY * CELL + CELL / 2 - 1, 2, 2, 0x2945);

                playerX = newX;
                playerY = newY;
                moves++;

                // Draw new position
                tft.fillRect(playerX * CELL + 2, playerY * CELL + 2, CELL - 4, CELL - 4, C_RED);

                // Update moves HUD
                tft.fillRect(0, 305, 120, 15, C_BLACK);
                tft.setTextColor(C_WHITE, C_BLACK);
                tft.drawString("Moves: " + String(moves), 5, 305, 2);

                if (g_app.soundOn) Sounds::sfxClick();
                delay(100);
            } else {
                // Hit wall
                if (g_app.soundOn) {
                    pinMode(PIN_BUZZER, OUTPUT);
                    digitalWrite(PIN_BUZZER, HIGH);
                    delay(30);
                    digitalWrite(PIN_BUZZER, LOW);
                }
                delay(80);
            }
        }

        // Win check
        if (playerX == goalX && playerY == goalY && !won) {
            won = true;
            isGameOver = true;
            if (g_app.soundOn) Sounds::sfxVictory();
            g_app.highScores[3] = max(g_app.highScores[3], (uint32_t)(1000 - moves));
        }
    }

    static void gameOverLoop() {
        if (won) {
            tft.fillScreen(C_BLACK);
            tft.setTextColor(C_GREEN, C_BLACK);
            tft.setTextSize(3);
            tft.drawCentreString("YOU ESCAPED!", 240, 100, 1);
            tft.setTextColor(C_YELLOW, C_BLACK);
            tft.setTextSize(2);
            tft.drawCentreString("Moves: " + String(moves), 240, 150, 1);
            tft.drawCentreString("Score: " + String(1000 - moves), 240, 180, 1);
        }
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.drawCentreString("Press A to Play Again", 240, 230, 1);
        if (g_input.btnAP || g_input.joyBtnP) {
            gameStarted = false;
            isGameOver = false;
            won = false;
        }
    }

    static void update() {
        if (g_input.btnCP && isGameOver) {
            gameStarted = false;
            isGameOver = false;
            won = false;
            return;
        }
        if (isGameOver) { gameOverLoop(); return; }
        play();
    }
}

#endif
