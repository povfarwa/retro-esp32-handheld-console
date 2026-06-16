#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include <vector>
#include "config.h"
#include "globals.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "nvs_save.h"

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

    tft.fillScreen(TFT_BLACK);
    spawnFood();
    for (auto& s : snake) {
        tft.fillRect(s.x + 1, s.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, TFT_GREEN);
    }
    tft.fillRect(food.x + 1, food.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, TFT_RED);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 13);
    tft.print("Score: 0");
}

static void play() {
    if (!gameStarted) { resetGame(); return; }

    Input::Axis ax = Input::axis();
    if (ax.x > 40 && dirX == 0)        { nextDirX = 1; nextDirY = 0; }
    else if (ax.x < -40 && dirX == 0)  { nextDirX = -1; nextDirY = 0; }
    else if (ax.y > 40 && dirY == 0)   { nextDirX = 0; nextDirY = 1; }
    else if (ax.y < -40 && dirY == 0)  { nextDirX = 0; nextDirY = -1; }

    if (Input::pressed(Input::RIGHT) && dirX == 0) { nextDirX = 1; nextDirY = 0; }
    if (Input::pressed(Input::LEFT)  && dirX == 0) { nextDirX = -1; nextDirY = 0; }
    if (Input::pressed(Input::BOTTOM) && dirY == 0) { nextDirX = 0; nextDirY = 1; }
    if (Input::pressed(Input::TOP)   && dirY == 0) { nextDirX = 0; nextDirY = -1; }

    unsigned long now = millis();
    if (now - lastMove < (unsigned long)moveDelay) return;
    lastMove = now;

    dirX = nextDirX; dirY = nextDirY;

    Point newHead = {snake[0].x + dirX * DOT_SIZE, snake[0].y + dirY * DOT_SIZE};

    if (newHead.x < 0) newHead.x = 480 - DOT_SIZE;
    else if (newHead.x >= 480) newHead.x = 0;
    if (newHead.y < 0) newHead.y = 320 - DOT_SIZE;
    else if (newHead.y >= 320) newHead.y = 0;

    snake.insert(snake.begin(), newHead);
    tft.fillRect(newHead.x + 1, newHead.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, TFT_GREEN);

    if (newHead.x == food.x && newHead.y == food.y) {
        score++;
        Sounds::sfxShoot();
        moveDelay = max(40, moveDelay - 2);
        spawnFood();
        tft.fillRect(food.x + 1, food.y + 1, DOT_SIZE - 2, DOT_SIZE - 2, TFT_RED);
    } else {

        Point tail = snake.back();
        tft.fillRect(tail.x, tail.y, DOT_SIZE, DOT_SIZE, TFT_BLACK);
        snake.pop_back();
    }

    for (size_t i = 1; i < snake.size(); i++) {
        if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
            isGameOver = true;
            Sounds::sfxGameOver();
            g_app.highScores[0] = max(g_app.highScores[0], (uint32_t)score);
            NVS::save();
            return;
        }
    }

    tft.fillRect(0, 0, 160, 20, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 13);
    tft.print("Score: ");
    tft.print(score);
}

static int gameOverChoice = 0;

static void drawOverlay() {

    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, TFT_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, TFT_RED);

    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(140, 80);
    tft.print("GAME OVER");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(150, 130);
    tft.print("Score: ");
    tft.print(score);

    int y1 = 185, y2 = 225;
    uint16_t colA = (gameOverChoice == 0) ? TFT_GREEN : TFT_DARKGREY;
    uint16_t colB = (gameOverChoice == 1) ? TFT_GREEN : TFT_DARKGREY;

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

void run() {
    gameStarted = false;
    isGameOver = false;
    score = 0;
    gameOverChoice = 0;
    tft.fillScreen(TFT_BLACK);

    Display::drawPanel(60, 80, SCREEN_W - 120, 160, TFT_DARKGREEN, TFT_GREEN, 12);
    Display::drawCentredText("SNAKE", 100, 3, TFT_GREEN);
    Display::drawCentredText("Eat food, grow long!", 140, 2, TFT_WHITE);
    Display::drawCentredText("Avoid yourself!", 165, 1, TFT_LIGHTGREY);
    Display::drawCentredText("Press SW to start", 195, 1, TFT_LIGHTGREY);
    Sounds::sfxSelect();
    while (true) {
        Input::update();
        if (Input::pressed(Input::SW) || Input::pressed(Input::RIGHT)) {
            Sounds::sfxClick(); delay(150);
            resetGame();
            break;
        }
        delay(16);
    }

    while (true) {
        Input::update();

        if (!isGameOver) {

            if (Input::pressed(Input::TOP)) {
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
                    Sounds::sfxClick();
                    drawOverlay();
                }

                if (Input::pressed(Input::SW)) {
                    Sounds::sfxSelect();
                    delay(150);
                    chosen = true;
                }

                if (Input::pressed(Input::TOP)) {
                    delay(150);
                    return;
                }

                delay(16);
            }

            if (gameOverChoice == 0) {

                resetGame();
            } else {

                return;
            }
        }

        delay(16);
    }
}

}

#endif
