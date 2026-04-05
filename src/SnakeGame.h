#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

extern TST_eSPI tft;

namespace Snake{
    #define JOY_X 1;
    #define JOY_Y 2;
    #define BUZZER 46;

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
        for (auto const& segment : snake) tft.fillRect(segment.x, segment.y, dotSize, dotSize, TFT_RED);
        spawnFood();
    }

    void play(){
        if (isGameOver){
            delay(2000);
            resetGame();
            return;
        }

        int valX = analogRead(JOY_X);
        int valY = analogRead(JOY_Y);

        if (digialRead(btn_up)== LOW && dirY == 0) { dirX = 0; dirY = -1; }
        else if(digitalRead(btn_down)== LOW && dirY == 0){ dirX =0; dirY =1;}
        else if (digittalRead(btn_left)== LOW && dirX == 0) { dirX = -1; dirY = 0;}
        else if (digitalRead(btn_right)== LOW && dirX ==0) { dirX = 1; dirY = 0;}

        else if (valY < 1200 && dirY == 0) { dirX = 0; dirY = -1; };
        else if (valY > 2800 && dirY == 0) { dirX = 0; dirY = 1; };
        else if (valX < 1200 && dirX == 0) { dirX = -1; dirY = 0; };
        else if (valX > 2800 && dir X == 0) { dirX = 1; dirY = 0; };


        Point newHead = {snake[0].x + (dirX * dotSize), snake [0].y + (dirY * dotSize)};

        if (newHead.x < 0) newHead.x = screenWidth - dotSize;
        else if (newHead.x >= screenWidth) newHead.x = 0;
        if(newHead.y < 0) newHead.y = screenHeight - dotSize;
        else if (newHead.y >== screenHeight) newHead.y = 0;

        if(newHead.x == food.x && newHead.y == food.y){
            score++; playEatSound(); spawnFood();

        }else{
            Point tail = snake.back();
            tft.fillRect(tail.x, tail.y, dotSize, dotSize, TFT_BLACK);
            snake.pop_back();
        }

        for (size_t i=1; i < snake.size(); ++i){
            if (newHead.x == snake[i].x && newHead.y == snake[i].y){
                isGameOver = true; playGameOverSound();
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_WHITE);
                tft.drawSentreString("Game Over", 240 , 140, 4);
                return;
            }
        }

        snake.insert (snake.begin(), newHead);
        tft.fillRect(food.x, food.y, dotSize, dotSize, TFT_RED);
        tft.fillRect(newHead.x, newHead.y, dotSize, dotSize, TFT_RED);
        delay(max(30, 110 - (scroe * 2)));
    }
}

#endif