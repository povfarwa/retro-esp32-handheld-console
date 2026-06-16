#ifndef DINO_RUN_H
#define DINO_RUN_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "config.h"
#include "globals.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "nvs_save.h"

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

static const uint16_t BROWN = 0xBE40;

static void reset() {
    tft.fillScreen(TFT_WHITE);
    tft.drawLine(0, groundY, 480, groundY, TFT_BLACK);
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

    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(380, 18);
    tft.print("HI: ");
    tft.print(highScore);
    tft.setCursor(10, 18);
    tft.print("Score: 0");
}

static void play() {
    if (!gameStarted) { reset(); return; }
    frameCount++;

    Input::Axis ax = Input::axis();
    if ((ax.y < -40 || Input::pressed(Input::RIGHT) || Input::pressed(Input::SW)) && !isJumping) {
        velocity = JUMP_FORCE;
        isJumping = true;
        Sounds::sfxShoot();
    }

    int oldY = dinoY;
    tft.fillRect(DINO_X - 8, oldY - 12, 40, 44, TFT_WHITE);

    if (oldY + 32 >= groundY) {
        tft.drawLine(0, groundY, 480, groundY, TFT_BLACK);
        tft.fillRect(DINO_X - 8, groundY + 1, 40, 30, TFT_WHITE);
    }

    velocity += GRAVITY;
    dinoY += (int)velocity;

    if (dinoY >= groundY - 25) {
        dinoY = groundY - 25;
        velocity = 0;
        isJumping = false;
    }

    if (millis() - lastSpawn > (unsigned long)random(800, 2000)) {
        obstacles.push_back({480, true});
        lastSpawn = millis();
    }

    for (auto& o : obstacles) {
        if (!o.active) continue;

        tft.fillRect(o.x - 5, groundY - 38, 24, 38, TFT_WHITE);
        o.x -= speed;
        if (o.x < -20) { o.active = false; continue; }

        tft.fillRect(o.x, groundY - 30, 12, 30, TFT_GREEN);
        tft.fillRect(o.x + 3, groundY - 36, 6, 10, TFT_GREEN);
        tft.fillRect(o.x - 4, groundY - 22, 6, 12, TFT_GREEN);
        tft.fillRect(o.x + 10, groundY - 18, 6, 8, TFT_GREEN);

        if (o.x - 4 < DINO_X + 18 && o.x + 16 > DINO_X &&
            dinoY + 25 > groundY - 36 && dinoY < groundY) {
            isGameOver = true;
            Sounds::sfxGameOver();
            highScore = max(highScore, score);
            g_app.highScores[4] = max(g_app.highScores[4], (uint32_t)score);
            NVS::save();
            return;
        }
    }

    tft.fillRect(DINO_X, dinoY, 18, 25, BROWN);
    tft.fillRect(DINO_X + 14, dinoY - 8, 14, 14, BROWN);
    tft.fillCircle(DINO_X + 22, dinoY - 4, 3, TFT_BLACK);

    if ((frameCount / 8) % 2 == 0) {
        tft.fillRect(DINO_X + 2, dinoY + 22, 5, 5, BROWN);
        tft.fillRect(DINO_X + 11, dinoY + 22, 5, 5, BROWN);
    } else {
        tft.fillRect(DINO_X + 2, dinoY + 25, 5, 5, BROWN);
        tft.fillRect(DINO_X + 11, dinoY + 25, 5, 5, BROWN);
    }
    tft.fillRect(DINO_X - 6, dinoY + 4, 6, 6, BROWN);

    if (millis() - lastScoreTime > 300) {
        score++;
        lastScoreTime = millis();
        tft.fillRect(0, 0, 100, 20, TFT_WHITE);
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
        tft.setCursor(10, 18);
        tft.print("Score: ");
        tft.print(score);

        if (score % 50 == 0) speed = min(14, speed + 1);
    }

    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), [](Obstacle& o) { return !o.active; }), obstacles.end());

    delay(20);
}

static int gameOverChoice = 0;

static void drawOverlay() {
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, TFT_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, TFT_RED);

    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(120, 70);
    tft.print("GAME OVER");

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(150, 115);
    tft.print("Score: ");
    tft.print(score);
    tft.setCursor(150, 140);
    tft.print("Best: ");
    tft.print(highScore);

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
    dinoY = 210;
    velocity = 0;
    isJumping = false;
    obstacles.clear();
    gameOverChoice = 0;
    tft.fillScreen(TFT_BLACK);

    Display::drawPanel(60, 70, SCREEN_W - 120, 180, TFT_MAROON, TFT_GREEN, 12);
    Display::drawCentredText("DINO RUN", 90, 3, TFT_GREEN);
    Display::drawCentredText("Jump over cacti!", 135, 2, TFT_WHITE);
    Display::drawCentredText("SW or RIGHT to jump", 170, 1, TFT_LIGHTGREY);
    Display::drawCentredText("Press SW to start", 210, 1, TFT_LIGHTGREY);
    Sounds::sfxSelect();

    while (true) {
        Input::update();
        if (Input::pressed(Input::SW) || Input::pressed(Input::RIGHT)) {
            Input::beep(25); delay(150);
            reset();
            break;
        }
        delay(16);
    }

    while (true) {
        Input::update();

        if (!isGameOver) {
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

}

#endif
