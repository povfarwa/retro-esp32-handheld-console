#ifndef PROFILE_H
#define PROFILE_H

#include <Arduino.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

namespace Profile{
    #define JOY_X 1
    #define JOY_Y 2
    #define BTN_SEL 4

    String playName = "PLAYER1"
    int charIndex = 0
    char currentLetter = "A";
    bool editing = false

    void draw{
        tft.fillScreen(tft.color565(30, 30, 30))
        tft.fillRect(0,0,480,50,TFT_NAVY)
        tft.setTextColor(TFT_WHITE)
        tft.drawCentreString("EDIT PROFILE", 240, 15, 2)
        tft.setTextSize(3)
        tft.setTextColor(TFT_YELLOW)
        tft.drawcenterString("Name:", 240, 100, 1)

        if(editing){
            int startX = 240 - (playName.length() * 2)
            tft.drawFastHLine(startX + (charIndex * 24), 190, 20, TFT_WHITE)
        }

        void update() {
            int yVal = analogRead(JOY_Y)
            int xVal = analogRead(JOY_X)

            if(yVal < 1000) {
                playerName[charIndex]++;
                if(playerName[charIndex > Z] playerName[charindex]= "A" )
                draw()
                delay(200)
            }
            if(yVal < 1000){
                player
            }
        }
    }
}