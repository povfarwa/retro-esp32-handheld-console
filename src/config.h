#pragma once

// ─────────────────────────────────────────────
//  Hardware Configuration
//  Board: ESP32-S3 DevKitC-1 N16R8
//  Display: ILI9488 3.5" 480×320 (TFT_eSPI)
// ─────────────────────────────────────────────

// Display
#define SCREEN_W  480
#define SCREEN_H  320

// Joystick
#define PIN_JOY_X   2
#define PIN_JOY_Y   1
#define PIN_JOY_SW  3
#define JOY_DEAD    200
#define JOY_MAX     4095

// Buttons (INPUT_PULLUP, active LOW)
#define PIN_BTN_RIGHT   4   // A
#define PIN_BTN_BOTTOM  5   // B
#define PIN_BTN_TOP     6   // C
#define PIN_BTN_LEFT    7   // D

// Buzzer (active LOW, digitalWrite HIGH = sound)
#define PIN_BUZZER  8

// Backlight
#define PIN_BACKLIGHT 21

// ── Game IDs ──
#define GAME_NONE          -1
#define GAME_SNAKE          0
#define GAME_SPACEINVADERS  1
#define GAME_PACMAN          2
#define GAME_BRICKBREAKER   3
#define GAME_DINORUN        4
#define GAME_TETRIS          5
#define GAME_COUNT          6

// LEDC channel for buzzer
#define BUZZER_LEDC_CHANNEL 0
