#include <Arduino.h>
#include <TFT_eSPI.h>
#include "globals.h"
#include "ui.h"
#include "menu.h"
#include "profile.h"
#include "settings.h"
#include "input.h"
#include "sounds.h"

// Game headers
#include "SnakeGame.h"
#include "SpaceShooter.h"
#include "FlappyBird.h"
#include "MazeRunner.h"
#include "DinoRun.h"
#include "RacingCar.h"

TFT_eSPI tft = TFT_eSPI();

// ─── Hardware TFT Reset ─────────────────────────────────────────
// Critical: gives the display a clean power-on reset sequence
static void hardwareResetTFT() {
    // TFT_RST is pin 8 (from platformio.ini build flag)
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, LOW);
    delay(20);
    digitalWrite(TFT_RST, HIGH);
    delay(50);
}

// ─── Force Backlight ON ─────────────────────────────────────────
static void enableBacklight() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}

// ─── Game Loop Helper ───────────────────────────────────────────
void runGameLoop(void (*updateFn)()) {
    // Back button (Btn C = top button, pin 6) exits to menu
    if (g_input.btnCP) {
        if (g_app.soundOn) Sounds::sfxBack();
        g_app.screen = Screen::HOME;
        Menu::setNeedsRedraw();
        delay(200);
        return;
    }
    updateFn();
}

// ─── Setup ──────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(0));

    // ── Critical: Initialize buzzer pin FIRST, keep it LOW ──
    // Pin 46 (NOT pin 8 — pin 8 is TFT_RST for the display!)
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    // ── Hardware display reset (required for reliable power-on) ──
    hardwareResetTFT();

    // ── Force backlight ON ──
    enableBacklight();

    // ── Initialize TFT ──
    tft.init();
    tft.setRotation(1);          // Landscape mode
    tft.fillScreen(TFT_NAVY);

    // ── Button inputs ──
    Input::init();

    // ── Initialize modules ──
    Menu::init();
    Settings::init();
    Profile::init();

    // ── Welcome screen ──
    tft.fillScreen(C_BG);        // Deep navy background
    tft.setTextColor(C_ACCENT, C_BG);
    tft.setTextSize(2);
    tft.drawCentreString("RETRO CONSOLE", 240, 100, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextSize(1);
    tft.drawCentreString("by Farwa Zafar", 240, 145, 2);
    tft.setTextColor(C_TEXT_DIM, C_BG);
    tft.drawCentreString("Press any button to start", 240, 200, 2);
    delay(2000);

    // ── Go to main menu ──
    g_app.screen = Screen::HOME;
    Menu::setNeedsRedraw();
}

// ─── Main Loop ──────────────────────────────────────────────────
void loop() {
    Input::poll();  // Capture joystick + button states every frame

    switch (g_app.screen) {
        case Screen::HOME:
            Menu::update();
            if (Menu::needsDraw()) Menu::draw();
            break;

        case Screen::SETTINGS:
            Settings::update();
            if (Settings::needsDraw()) Settings::draw();
            break;

        case Screen::PROFILE:
            Profile::update();
            if (Profile::needsDraw()) Profile::draw();
            break;

        // ── 6 GAMES ──
        case Screen::GAME_SNAKE:   runGameLoop(SnakeGame::update);   break;
        case Screen::GAME_SPACE:   runGameLoop(SpaceShooter::update); break;
        case Screen::GAME_FLAPPY:  runGameLoop(FlappyBird::update);   break;
        case Screen::GAME_PUZZLE:  runGameLoop(MazeRunner::update);   break;
        case Screen::GAME_ASTEROID:runGameLoop(DinoRun::update);      break;
        case Screen::GAME_BRICK:   runGameLoop(RacingCar::update);    break;

        default:
            g_app.screen = Screen::HOME;
            Menu::setNeedsRedraw();
            break;
    }

    delay(10);  // Frame stabilizer
}
