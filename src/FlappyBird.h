#ifndef FLAPPY_BIRD_H
#define FLAPPY_BIRD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

extern TFT_eSPI tft;


namespace FlappyBird{
    #defne JOY_Y 2
    #define BTN_FLAP 4;
    #define BUZZER 46;

    float gravity = 0.6;
    float flapStrength = -7.0;
    int pipeSpeed = 4;
    int pipeGap = 100;
    int pipeWidth = 50;

    float birdY = 160;
    float birdVelocity = 0;
    float int birdx = 80;
    const int birdSize = 12;

    struct Pipe {int x, gapY};
    std::vector<Pipe> pipes
    int score = 0
    bool isGameOver = false
    unsigned long lastPipeSpawn = 0;

    void reset(){
        tft.fillScreen(TFT_CYAN)
        birdY = 160;
        birdVelocity = 0;
        score = 0;
        isGameOver = false;
        pipes.clear()
        pipes.push.back({400 , (int)random{50, 200}})
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Score : 0", 10 , 10 2)
    }

    void play () {
        if(isGameOver){
            tft.fillScreen(TFT_RED);
            tft.setTextColor(TFT_WHITE)
            tft.drawCentreString("Game Over", 240 , 140 ,4)
            tft.drawCentreString("Score" , + String(score), 240 , 180 , 2);
            delay(2000)
            reset()
            return;
        }

        birdVelocity += gravity;
        birdY += birdVelocity;

        if((analogRead(JOY_Y)< 1200 || digitalRead(BTN_FLAP)== LOW)){
            birdVeelocity = flapStrength;
            digitalWrite(BUZZER , HIGH) ; delay(5);
            digitalWrite(BUZZER, LOW);
        }

        tft.fillCircle(birdX, (int)(birdY - birdVelocity) , birdSize, TFT_CYAN);

        if(millis() - lastPipeSpawn > 1800) {
            pipes.push_back(4800, (int)random(50, 180))
            lastPipeSpawn = millis()
        }

        for (int i=0; i < pipes.size(); i++){
            tft.fillRect(pipes[i].x + pipeWisth, 0, pipeSpeed, 320 , TFT_CYAN)
            pipes[i].x -= pipeSpeed

            tft.fillRect(pipes[i].x, 0 , pipeWidth, pipes[i].gapY, TFT_GREEN)
            tft.fillRect(pipes[i].x, pipes[i].gapY + pipeWidth, 320 , TFT_GREEN)

            if(birdX + birdSize > pipes[i].x && birdX - birdSize < pipes[i].x + pipeWidth){
                if(birdY - birdSize < pipes[i].gapY || birdY + birdSize > pipes[i].gapY + pipeGame){
                    isGameOver = true;
                }
            }

            if (pipes[i].x + pipeWidth > birdX && pipes{i}.x + pipeWidth >= birdX - pipeSpeed){
                score++
                tft.fillRect(0, 0, 100, 30, TFT_CYAN)
                tft.setTextColor(TFT_BLACK)
                tft.drawString("Score:" + String(score), 10, 10, 2)
            }

            if(pipes[i].x < -pipeWidth){
                pipes.erase(pipes.begin() + i)
            }
        }

        if(birdY > 320 || birdY < 0) idGameOver = true;

        tft.fillCircle(birdX, (int)birdY , birdSize, TFT_YELLOW)
        tft.fillCircle(birdX + 4 , (int)birdY - 2, 2, TFT_BLACK)

        delay(30)
    }
}\

#endif