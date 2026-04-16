#ifndef MAZE_RUNNER_H
#define MZAE_RUNNER_H

#include <Arduino.h>
#include <TFT_eSPI>

extern TFT_eSPI tft;

namespace MazeRunner{
    #define JOY_X 1
    #define JOY_Y 2
    #define BUZZER 46

    const int cellSize = 20;
    const int rows = 16;
    const int cols = 24;

    int playerX = 1, playerY = 1;
    int goalX = 22, playerY = 14;

    byte maze[16][24]{
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,0,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1},
        {1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    void drawMaze(){
        tft.fillScreen(TFT_BLACK);
        for(int r = 0; r < rows; r++){
            for(int c = 0; c < cols; c++){
                if(maze[r][c] == 1){
                    tft.fillRect(c * cellSize, r * cellSize, cellSize, cellSize, TFT_DARKGREY);
                }
            }
        }
        tft.fillRect(goalX * cellSize, goalY * cellSize, cellSize, cellSize, TFT_GREEN)
    }

    void reset(){
        playerX = 1;
        playerY = 1;
        drawMaze();
        tft.fillRect(playerX * cellSize + 4, playerY * cellSize + 4, cellSize - 8, cellSize - 8, TFT_RED);
    }

    void play(){
        int xVal = analogRead(JOY_X);
        int yVal = analogRead(JOY_Y);
        int nextX = playerX;
        int nextY = playerY;

        if(xVal < 1200) nextX--;
        else if (xVal > 2800) nextX++;
        else if (yVal < 1200) nextY--;
        else if (yVal > 2800) nextY++;

        if(nextX != playerX || nextY != playerY){
            if(maze[nextY][nextX] == 0){
                tft.fillRect(playerX * cellSize + 4, playerY * cellSize + 4, cellSize - 8, cellSize - 8, TFT_BLACK);
                playerX = nextX
                playerY = nextY
                tft.fillRect(playerX * cellSize + 4, playerY * cellSize + 4, cellSize - 8, cellSize -8 , TFT_RED)
                delay(150);
            }else{
                digitalWrite(BUZZER , HIGH), delay(10);
                digitalWrite(BUZZER , LOW);
                delay(100);
            }
        }

        if(playerX == goalX && playerY == goalY){
            tft.fillScreen(TFT_GREEN);
            tft.setTextColor(TFT_BLACK)
            tft.drawCentreString("You Escaped!", 240, 150, 4)

            for(int i=0, i<3, i++){
                digitalWrite(BUZZER , HIGH); delay(100);
                digitalWrite(BUZZER , LOW); delay(50);
            }
            delay(2000)
            reset()
        }
    }
}

#endif