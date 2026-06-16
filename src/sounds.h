#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────
//  Sound module — active buzzer
//  Just digitalWrite HIGH/LOW timing.
// ─────────────────────────────────────────────

namespace Sounds {

    void init(uint8_t pin);

    void tone(uint32_t freq, int durationMs);  // freq ignored for active buzzer
    void silence(int durationMs);

    // Effects
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
