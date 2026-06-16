#include "sounds.h"

namespace Sounds {

static uint8_t _pin = 0;

static void _beep(int onMs) {
    digitalWrite(_pin, HIGH);
    delay(onMs);
    digitalWrite(_pin, LOW);
}

static void _gap(int ms) {
    delay(ms);
}

void init(uint8_t pin) {
    _pin = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void tone(uint32_t freq, int durationMs) {
    _beep(durationMs);
}

void silence(int durationMs) {
    _gap(durationMs);
}

void sfxClick()      { _beep(12); }
void sfxSelect()     { _beep(40); _gap(20); _beep(80); }
void sfxBack()       { _beep(80); _gap(20); _beep(40); }
void sfxBrickHit()   { _beep(15); _gap(8);  _beep(15); }
void sfxPaddleHit()  { _beep(25); }
void sfxLifeLost()   { _beep(120); _gap(40); _beep(120); _gap(40); _beep(250); }
void sfxGameOver()   { _beep(100); _gap(40); _beep(100); _gap(40); _beep(100); _gap(40); _beep(400); }
void sfxVictory()    { _beep(80); _gap(30); _beep(80); _gap(30); _beep(80); _gap(30); _beep(300); _gap(60); _beep(80); _gap(20); _beep(400); }
void sfxShoot()      { _beep(20); _gap(10); _beep(10); }
void sfxExplosion()  { for (int i = 0; i < 5; i++) { _beep(30); _gap(15); } _beep(80); }
void sfxEnemyHit()   { _beep(20); _gap(10); _beep(20); }
void sfxPlayerHit()  { _beep(80); _gap(30); _beep(80); _gap(30); _beep(150); }
void sfxLevelUp()    { for (int i = 0; i < 4; i++) { _beep(60); _gap(30); } _beep(200); }
void sfxPowerUp()    { for (int i = 0; i < 6; i++) { _beep(40 + i*20); _gap(15); } _beep(300); }

void jingleStartup() {
    _beep(80);  _gap(40);
    _beep(80);  _gap(40);
    _beep(80);  _gap(80);
    _beep(80);  _gap(40);
    _beep(120); _gap(40);
    _beep(200); _gap(60);
    _beep(200);
}

}
