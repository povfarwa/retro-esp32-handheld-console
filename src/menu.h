#pragma once

// ─────────────────────────────────────────────
//  Main menu
//  Returns the GAME_* ID of the selected game.
//  Special: returns -2 for Settings, -3 for Profile
// ─────────────────────────────────────────────

namespace Menu {
    void init();
    int  run();   // blocks until selection; returns GAME_* ID or -2/-3
}
