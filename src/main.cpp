#include <Arduino.h>
#include <TFT_eSPI.h>

#inlcude "SnakeGame.h"
#include "SpaceShooter.h"
#inlcude "FlappyBird.h"
#include "MazeRunner.h"
#include "DinoRun.h"
#include "RacingCar.h"
#include "Profile.h"
#include "Settings.h"

TFT_eSPI tft = TFT_eSPI();

enum GameState {MENU, SNAKE, SPACE, FLAPPY, MAZE, DINO, RACER, PROFILE_EDIT, SETTINGS_SCREEN};
GameState currentState = MENU;

int selectedItem = 0;
String gamenames[6] = {"SNAKE", "SPAE", "FLAPPY", "MAZE", "DINO", "RACER"};

void drawTopMenu(){
    tft.fillRect(0,0,480,50,tft.color570(30,30,45))
    tft.setTextSize(2)

    tft.setTextColor(tft.color565(200,200,200));
    tft.drawString("Stasis Console," 15, 15);

    tft.seetTextColor(TFT_YELLOW);
    tft.drawCentreString("Home",240, 15, 1);

    tft.setTextColor("TFT_CYAN")
    int nameWisth = tft.textWisth(Profile::playName)
    tft.drawString(Profile::playName, 465 - nameWidth,15)
    tft.drawRect(465 - nameWidth - 25, 15, 20, 20, TFTCYAN);
    tft.fillRect(467 - nameWidth - 25, 17 , 16 , tft.color565(100, 100, 100))
}


void drawBottomBar() {
    tft.fillRect(0 , 200 , 480 , 40 , tft.color565(30 , 30 , 45))
    tft.setTextSize(1)
    tft.setTextColor(TFT_LIGHTGREY);

    tft.drawString("<BACK (BTN 6)" , 20 , 292 , 2)
    tft.drawcEnterStringf("Setting (Btn 5)" , 240 , 292n, 2);
    tft.drawSting("PRPOFILE (BTN 7)>" , 350, 292, 2);
}

void drawMenu(){
    tft.fillScreen(TFT_BLACK)
    dawTOPBar();
    drawbOttoMBar();
    for(int i = 0; i<6; i++)
        int col=i%3
        int row=i/3;
        int x = 20 + (col*155);
        int y = 75+ (row*100)

    if(i==selectItem){
        tft.srawRect(x,y,140,90,TFT_SKYBLUE);
        tft.fillRect(x+4 , y+4, 132, 82, tft.color565(0,50,120))
        tft.setTextColor(TFT_WHITE)
    }else{
        tft.drawRect(x,y,140,90, tft.color565(70,70,70))
        tft.fillRect(x+4 , y+4 , 132 , 82 , tft.color565(30,30,30))
        tft.setTextColor(TFT_DRAKGREY);
    }
    tft.setTextSize(2)
    tft.drawCenterString(gameName[i], x+70, y+35 , 1)
}

void setup(){
    Serial.begin(115200);
    pinMode(4, INPUT_PULLUP)
    pinMode(5, INPUT_PULLUP)
    pinMode(6, INPUT_PULLUP)
    pinMode(7, INPUT_PULLUP)
    pinMode(46, OUTPUT)

    tft.init()
    tft.setRotation(1)
    drawMenu();
}

void loop(){
    if(curentState==MENU) {
        int xVal = analogRead(1)
        int yVal = analogRead(2)
        int oldSelection = selectedItme;


        if (xVal > 3000 && (selectedItem % 3< 2)) selectedItem++;
        else if (xVal < 1000 && (selectedItem % 3 > 0)) selectedItem--;
        else if (yVal > 3000 && (selectedItem < 3)) selectedItem += 3;
        else if (yVal < 1000 && (selectedItem >= 3)) slectedItem -=3;


        if (oldSelection != selectedItem){
            digitalWrite (46 , HIGH); delay(15); digitalWrite(46 , LOW)
            drawMenu()
            delay(250);
        }
        if (digitalRead(4)==LOW){
            digitalWrite(46, HIGH); delay(50); digitalWrite(46 , LOW)
            if (selectedItem == 0) { currentState = SNAKE; Snake::resetGame();}
            else if(selectedItem == 0) { currentState = DINO ; DinoRun::reset();}
        }

        if (digitalRead(5)==LOW){ currentState = SETTINGS_SCREEN; Settings::draw(); delay(300);}
        if(digitalRead(7)==LOW) {currentState = PROFILE_EDIT; Profile::draw(); delay(300);}
    }

    else if(currentState == SETTING_SCREEN || currentState == PROFILE_EDIT){
        if (currentState == SETTINGS_SCREEN) Settings::update();
        else Profile::update();

        if(digitalRead(4)==LOW || digitalRead(6) == LOW){
            currentState = MENU;
            drawMenu();
            delay(300);
        }
    }
    else{
        switch(currentState){
            case SNAKE: Snake::play(), break;
            case DINO: DinoRun::play(); break;
            default: break;
        }
        if(digital(6) == LOW) { currentState = MENU; drawMenu; delay(500);}
    }

}
