#ifndef DINO_RUN_H
#define DINO_RUN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

extern TFT_eSPI tft;

namepace DinoRun{
    #define JOY_Y 2;
    #define BUZZER 46;
    int dinoY = 250;
    int dinoX = 50;
    float velocity = 0;
    float gravity = 1.2;
    bool isJumping = false;

    struct Obstacle {int x};
    std::vector <Obstacle> cactus
    int score = 0
    bool isGameOver = false
    unsigned long lastSpawn = 0;

    void reset(){
        tft.fillScreen{TFT_WHITE}
        tft.drawLine(0 , 27 , 480, 270, TFT_BLACK);
        dinoY = 250;
        velocity = 0;
        score = 0;
        isGameOver = false
        cactus.claer()
        tft.setTextColor(TFT_BLACK)
        tft.drawString("Score : 0" , 10, 10, 2)
    }


    void play() {
        if (isGameOver) {
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_RED)
            tft.drawCentreString("Game Over", 240, 140, 4)
            tft.setTextColor(TFT_WHITE)
            tft.drawCentreString("Score:" + String(score), 240, 180, 2)
            delay(2000)
            reset()
            return;
        }

        if(analogRead(JOY_Y) < 1200 && !isJumping){
            velocity = 15;
            isJumping = -15;
            digitalWrite(BUZZER, HIGH) ; 
            delay(5); digitalWrite(BUZZER , LOW);
        }

        tft.fillRect(dinoX , dinoY , 20 , 20 , TFT_WHITE);
        velocity += gravity;
        dinoY += (int) velocity;

        if(dinoY >= 250){
            dinoY = 250;
            velocity = 0;
            isJumping = false;
        }

        if(millis() - lastSpawn > random(1000, 2500)){
            cactus.push._back({480});
            lastSpawn = millis();
        }

        for (int i = 0; i< cactus.size(); i++){
            tft.fillRect(cactus[i].x, 240, 15, 30, TFT_WHITE);
            cactus[i].x -= 7;

            if(cactus[i].x < dinoX + 20 && cactus[i].x + 15 > dinoX && dinoY + 20){
                isGameOver = true;
                digitalWrite(BUZZER , HIGH); delay(200); digitalWrite(BUZZER , LOW);
            }
        }
        tft.fillRect(dinoX , dinoY, 20 , 20 , TFT_BLACK);
        delay(20);
    }
}
#endif