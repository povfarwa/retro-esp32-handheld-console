#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "menu.h"
#include "settings.h"
#include "profile.h"

// Games
#include "SnakeGame.h"
#include "game_spaceinvaders.h"
#include "TetrisGame.h"
#include "game_brickbreaker.h"
#include "DinoRun.h"
#include "SamuraiFight.h"

void setup() {
    Serial.begin(115200);

    Input::init();         // buttons + joystick
    Sounds::init(PIN_BUZZER);
    Display::init();       // TFT_eSPI display
    Settings::init();      // load NVS
    Profile::init();       // load NVS
    Menu::init();

    Sounds::jingleStartup();
}

void loop() {
    int game = Menu::run();

    switch (game) {
        case GAME_SNAKE:         SnakeGame::run();         break;
        case GAME_SPACEINVADERS: SpaceInvaders::run();     break;
        case GAME_PACMAN:        SamuraiFight::run();       break;
        case GAME_BRICKBREAKER:  BrickBreaker::run();      break;
        case GAME_DINORUN:       DinoRun::run();           break;
        case GAME_TETRIS:        Tetris::run();            break;
        case -2:                 Settings::run();           break;
        case -3:                 Profile::run();            break;
        default:                                           break;
    }

    // Redraw menu after returning from game/settings/profile
    Menu::init();
}
