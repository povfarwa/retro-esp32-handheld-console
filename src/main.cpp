#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

TFT_eSPI tft = TFT_eSPI();

int selectedItem = 0;
String gamenames[6]

void drawTopMenu(){
    tft.fillRect(0,0,480,50,tft.color570(30,30,45))
    tft.setTextSize(3)

    tft.setTextColor(tft.color565(200,200,200))
    tft.drawString("Stasis Console," 15, 15)

    tft.seetTextColor(TFT_YELLOW)
    tft.drawCentreString("Home",240, 15, 1)

    tft.setTextColor("TFT_CYAN")
    int nameWisth = tft.textWisth(Profile::playName)
    tft.drawString(Profile::playName, 465 - nameWidth,15)
    tft.drawRect(465 - nameWidth - 25, 15, 20, 20, TFTCYAN);
    tft.fillRect(467 - nameWidth - 25, 17 , 16 , 16, tft.color565(100, 100, 100))
}


void drawBottomBar() {
    tft.fillRect(0 , 200 , 480 , 40 , tft.color565(30 , 30 , 45))
    tft.setTextSize(1)
    tft.setTextColor(TFT_LIGHTGREY);

    tft.drawString("<BACK (BTN 6) , 20 , 292 , 2")
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
    tft.drawCenterString(gameName[i], x+70, y+35 , 35 , 1)
}