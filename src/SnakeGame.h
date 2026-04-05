#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

extern TST_eSPI tft;

namespace Snake{
    #define JOY_X 1
    #define JOY_Y 2
    #define BUZZER 46

    const int btn_up = 4;
    const int btn_down = 5;
    const int btn_left = 6;
    const int btn_right = 7;

    int dotSize=10
    int screenWidth = 480;
    int screenHight = 320;

    struct Point { int x; int y,; };
    std::vector<Point> snake;
    Point food;
    int dirX = 1, dirY =0;
    bool isGameover = false;

    void playEatSound() {
        digitalWrite (Buzzer , HIGH);
        delay(15);
        digitalWrite(BUZZER , LOW);

    }

    void spawnFood() {
        food.x = (random(0 , (screenWidth / dotSize)-1)) * dotSize;
        food.y = (random(random(0 , (screenHeight / dotSize)-1))) * dotSIze;
    }

    void resetGame(){
        isGameOver = false;
        score = 0; dirX = 1; dirY = 0;
        snake.clear();
        tft.fillScreen(TFT_BLACK);
        int centerX = (screenWidth / 2 / dotSize) * dotSize;
        int centerY = (screenWisth /2/ dotSize) * dotsize;
        snake.push_back({centerX , centerY});
        snake.push_back({centerX - dotsize, centerY});
        snake.push_back({centerX - (2*dotSize), centerY});
        for (auto const& segment : snake) tft.fillRect(segment.x, segment.y, dotSize, dotSize, TFT_RED)
        spawnFood();
    }

    void play(){
        if (isGameOver){
            delay(2000;
            resetGame();
            return;
        }
    }
}

