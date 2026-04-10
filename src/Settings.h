#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <TFT_eSPI>

extern TFT_eSPI;

namespace Settings{
    int brightness = 80
    const int ledPin = 21

    void draw() {
        tft.fillScreen(tft.color565(20, 20, 20));
        tft.fillRect(0, 0, 480, 50, tft.color565(50, 50, 70))
        tft.setTextColor(tFT_WHITE)
        tft.drawCentreString("SYSTEM SETTINGS", 240, 15, 2)
        tft.setTextColor(TFT_YELLOW)
        tft.drawCentreString("SCEEN SETTINGS", 240, 100, 2)
        tft.drawRect(100, 150, 280, 30, TFT_WHITE)
        tft.fillRect(102, 152, (brightness * 2.76), 26, TFT_CYAN)
        tft.setTextColor(TFT_WHITE)
        tft.drawCenterString(string(brightness) + "%", 240, 190, 2)
        tft.setTestSize()
        tft.drawCentreString("JOYSTICK LEFT/RIGHT TO ADJUST", 240, 250, 2)
    }

    void update(){
        int xVal =analogRead(1);
        if(xVal < 1000 && brightness > 10){
            brightness -= 10;
            draw()
            analogWrite(ledPin, map(brightness, 0, 100, 0, 255))
            delay(200)
        }
        if(xVal > 3000 && brightness < 100){
            brightness += 10;
            draw()
            analogWrite(ledPin, map(brightness, 0, 100, 0, 255))
            delay(200)
        }
    }
}

#endif