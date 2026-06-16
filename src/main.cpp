#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "menu.h"
#include "settings.h"
#include "profile.h"

#include "SnakeGame.h"
#include "game_spaceinvaders.h"
#include "TetrisGame.h"
#include "game_brickbreaker.h"
#include "DinoRun.h"
#include "PacMan.h"

void setup() {
    Serial.begin(115200);

    Input::init();
    Sounds::init(PIN_BUZZER);
    Display::init();
    Settings::init();
    Profile::init();
    Menu::init();

    Sounds::jingleStartup();
}

void loop() {
    int game = Menu::run();

    switch (game) {
        case GAME_SNAKE:         SnakeGame::run();         break;
        case GAME_SPACEINVADERS: SpaceInvaders::run();     break;
        case GAME_PACMAN:        PacMan::run();             break;
        case GAME_BRICKBREAKER:  BrickBreaker::run();      break;
        case GAME_DINORUN:       DinoRun::run();           break;
        case GAME_TETRIS:        Tetris::run();            break;
        case -2:                 Settings::run();           break;
        case -3:                 Profile::run();            break;
        default:                                           break;
    }

    Menu::init();
}
