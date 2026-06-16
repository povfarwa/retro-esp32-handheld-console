#pragma once
#include <Arduino.h>

namespace Sounds {

    void init(uint8_t pin);

    void tone(uint32_t freq, int durationMs);
    void silence(int durationMs);

    void sfxClick();
    void sfxSelect();
    void sfxBack();
    void sfxBrickHit();
    void sfxPaddleHit();
    void sfxLifeLost();
    void sfxGameOver();
    void sfxVictory();
    void sfxShoot();
    void sfxExplosion();
    void sfxEnemyHit();
    void sfxPlayerHit();
    void sfxLevelUp();
    void sfxPowerUp();

    void jingleStartup();
}
