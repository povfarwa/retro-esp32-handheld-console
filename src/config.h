#pragma once

#define SCREEN_W  480
#define SCREEN_H  320

#define PIN_JOY_X   2
#define PIN_JOY_Y   1
#define PIN_JOY_SW  3
#define JOY_DEAD    200
#define JOY_MAX     4095

#define PIN_BTN_RIGHT   4
#define PIN_BTN_BOTTOM  5
#define PIN_BTN_TOP     6
#define PIN_BTN_LEFT    7

#define PIN_BUZZER  8

#define PIN_BACKLIGHT 21

#define GAME_NONE          -1
#define GAME_SNAKE          0
#define GAME_SPACEINVADERS  1
#define GAME_PACMAN          2
#define GAME_BRICKBREAKER   3
#define GAME_DINORUN        4
#define GAME_TETRIS          5
#define GAME_COUNT          6

#define BUZZER_LEDC_CHANNEL 0
