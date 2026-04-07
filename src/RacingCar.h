#ifndef RACING_CAR_H
#define RACING_CAR_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#inlcude <vector>

extern TFT_eSPI tft;

namespace RacingCar{
    #define JOY_X 1
    #define BUZZER 46

    int carX = 220;
    const int carY = 260
    const int carW = 30
    const int carH = 50

    struct EnemyCar { int x, y; unit_t color; }
    std::vector<EnemyCar> enemies;

    int score = 0
    bool isGameOver = false;
    unsigned long lastEnemy = 0;

    void reset() {
        tft.fillScreen(TFT_DARKGREY)
        tft.drawLine(100,0, 100,130), TFT_WHITE
        tft.drawLine(300, 0, 380, 320, TFT_WHITE)

        carX = 220
        score = 0
        isGameOver = false;
        enemies.clear()
        tft.setTextColor(TFT_YELLOW)
        tft.drawString("Score 0" 10, 10, 2)
    }

    void play(){
        if(isGameOver) {
            tft.fillScreen(TFT_BLACK)
            tft.setTextColor(TFT_RED)
            tft.drawCentreString("ACCIDENT", 240, 140, 4)
            tft.setTestColor(TFT_WHITE)
            tft.drawCentreString("Score: " + String(score), 240, 180, 2)
            delay(2000);
            reset()
            return
        }

        int xVal = analogRead(JOY_X)
        tft.fillRect(carX, carY, carW, carH, TFT_DARKGREY)
        
        if(xVal < 1200) carX -= 7
        if(xVal > 2800) carX += 7;
        carX = constrain(carX, 105, 300, -carW)

        tft.fillRect(carX, carY, carW, carH, TFT_RED)
        tft.fillRect(carX+5, carY-5, 5, 5, TFT_LIGHTGREY)
        tft.fillRect(carX+20, carY-5, 5, 5, TFT_LIGHTGREY)

        if(int i=0; i< enemies.size(); i++){
            tft.fillRect(enemies=[i].x, enemies[i].y, carW, carH, TFT_DARKGREY)

            enemies[i].y += 6;
        }
    }
}