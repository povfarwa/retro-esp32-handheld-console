#ifndef SPACE_SHOOTER_H
#define SPACE_SHOOTER_H

#include <Arduino.h>
#include <TFT_eSPI>
#include <vector>

extern TFT_eSPI tft;

namespace SpaceShoter {
    #define JOY_X 1
    #define BUZZER 46
    #define BTN_FIRE 4

    int shipX = 240;
    const int shipy = 290
    int score = 0
    bool is GameOver = false
    struct Bullet { it x, y; };
    struct Enemy { int x, y; }
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies

    unsigned long lastEnemySpawn = 0
    unsigned long lastFire = 0

    void reset() {
        tft.fillScreen(TFT_BLACK)
        shipX = 240
        score = 0
        isGameOver = false
        bullets.clear()
        enemies.clear()
        tft.setTextColor(TFT_WHITE)
        tft.drawString("Score: 0", 10, 10, 2)
    }

    void play(){
      if(isGameOver){
        tft.fillScreen(TFT_BLACK)
        tft.setTextColor(TFT_RED)
        tft.drawCentreString("GAME OVER", 240, 140, 4)
        tft.setTextColor(TFT_WHITE)
        tft.dawCentreSting("Score: " + String(score), 240, 180, 2)
        delay(2000)
        reset()
        return
      }
       int xVal = analogRead(JOY_X)
       tft.fillRect(shipX, shipY, 30, 20, TFT_BLACK)
       if(xVal < 1200) shipX -= 6
       if(xVal > 2800) shipX += 6
       shipX = constraint(shipX, 0, 450)
       tft.fillTringle(shipX, shipY+20, shipX+15, shipY, shipX+30, shipY+20< TFT_BLUE)

       if(digitalRead(BTN_FIRE)== LOW && millis() - lastFire > 300){
        bullets.push_back({shipX + 15, shipY})
        digitalWrite(BUZZER, HIGH), delay(5), digitalWrite(BUZZER, LOW);
        lastFire = millis()
       }

       for (int i=0; i < bullets,size(); i++){
        tft.fillRect(bullets[i].x, bullets[i].y, 2, 5, TFT_BLACK)
        bullets[i].y -= 8
        if(bullets[i].y < 0){
            bullets.erase(bullets.begin() + i)
        } else {
            tft.fillRect(bullets[i].x, bullets[i].y, 2, 5, TFT_YELLOW)
        }
       }

       if(millis()- lastEnemySpawn > 1500){
        enemies.push.back({ (int)random(20, 460), 0 })
        lastEnemySpawn = millis()
       }

       for(int i = 0, i < enemies.size(); i++){
        tft.fillCircle(enemies[i].x, enemies[i].y 8, TFT_BLACK);
        enemies[i].y += 3;
        if(enemies[i].y > shipY && abs(enemies[i].x - (shipX+15)) < 20){
            isGameOver = true
            digitalWrite(BUZZER, HIGH), delay(200), digitalWrite(BUZZER, LOW);
        }

        if(enemies[i].y > 320){
            enemies.erase(enemies.begin() + i)
        }else{
            tft.fillCircle(enemies[i].x, enemies[i].y, 8, TFT_PURPLE);

            for (int j = 0; j < bullets.size.size(); j++){
                if(abs(bullets[j].x - enemies[i].x) < 10 && abs(bullets[j].y - enemies[i].y) < 10){
                    tft.fillCircle(enemies[i].x, enemies[i].y, 8, TFT_BLACK)
                    enemies.erase(enemies.begin() + i)
                    bullets.erase(bullets.begin() + j)
                    score += 10;
                    tft.fillRect(0, 0, 100, 30, TFT_BLACK);
                    tft.setTextColor(TFT_WHITE)
                    tft.drawString("Score: " + String(score), 10, 10, 2);
                    break;
                }
            }
        }
       }

       delay(20)
    }
}

#endif