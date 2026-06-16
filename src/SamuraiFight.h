#ifndef SAMURAI_FIGHT_H
#define SAMURAI_FIGHT_H

#include <Arduino.h>
#include <algorithm>
#include "config.h"
#include "globals.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "nvs_save.h"

namespace SamuraiFight {

// ── Constants ──
static const int MAX_HP    = 100;
static const int MAX_ENERGY = 100;

// ── Colors ──
static const uint16_t C_BG        = 0x2108; // dark dojo
static const uint16_t C_FLOOR     = 0x6A4B; // wooden floor
static const uint16_t C_PLAYER    = 0x001F; // blue
static const uint16_t C_PLAYER2   = 0x7DFF; // light blue
static const uint16_t C_HP_BAR    = 0x07E0; // green
static const uint16_t C_HP_LOW    = 0xF800; // red
static const uint16_t C_ENERGY_BAR= 0x06FF; // cyan
static const uint16_t C_ENERGY_BG = 0x2124;

struct EnemyInfo {
    const char* name;
    int hp;
    int dmgMin, dmgMax;
    int atkSpeed;   // lower = faster
    uint16_t color;
    uint16_t color2;
};

static const EnemyInfo ENEMIES[] = {
    { "BANDIT",     50,  4,  8,  80,  0x8C44, 0xBE60 },
    { "NINJA",      55,  5, 10,  60,  0x0000, 0x3186 },
    { "RONIN",      70,  6, 12,  70,  0x738E, 0x9CF3 },
    { "ONI",        85,  8, 14,  75,  0xF800, 0xFD20 },
    { "SHADOW",     90,  8, 16,  55,  0x780F, 0xB01F },
    { "DARK LORD", 110, 10, 20,  65,  0x7800, 0xF800 },
    { "SAMURAI",   130, 12, 22,  60,  0x001F, 0x07FF },
};
static const int ENEMY_COUNT = sizeof(ENEMIES) / sizeof(ENEMIES[0]);

// ── State ──
static int playerHP, playerMaxHP;
static int playerEnergy;
static int enemyHP;
static int level;
static int enemyIdx;
static int playerBlocking;
static unsigned long blockUntil;
static int flashOverlay;     // frames remaining for hit flash (0=none)
static uint16_t flashColor;  // color of flash overlay
static unsigned long lastAtkTime;
static unsigned long lastEnemyAtkTime;
static unsigned long lastEnergyTime;
static int screenShake;
static int shakeDur;
static bool gameOver;
static bool gameStarted;
static int gameOverChoice;
static int battleMsgTimer;
static String battleMsg;
static int enemyAtkCooldown;
static int playerAtkCooldown;
static int playerAnim;
static int enemyAnim;
static unsigned long lastAnimTime;
static int playerX, enemyX;
static float playerSwordAngle;
static float enemySwordAngle;

// ── Helpers ──
static int enemyLevelIdx(int lvl) {
    if (lvl <= 2) return 0;
    if (lvl <= 4) return 1;
    if (lvl <= 6) return 2;
    if (lvl <= 8) return 3;
    if (lvl <= 10) return 4;
    if (lvl <= 13) return 5;
    return 6;
}

static const EnemyInfo& currentEnemy() {
    return ENEMIES[enemyIdx];
}

static void showMessage(const String& msg) {
    battleMsg = msg;
    battleMsgTimer = 30;
}

static void reset() {
    playerHP = MAX_HP;
    playerMaxHP = MAX_HP;
    playerEnergy = MAX_ENERGY;
    level = 1;
    enemyIdx = enemyLevelIdx(level);
    enemyHP = currentEnemy().hp;
    playerBlocking = 0;
    blockUntil = 0;
    lastAtkTime = 0;
    lastEnemyAtkTime = 0;
    lastEnergyTime = millis();
    screenShake = 0;
    shakeDur = 0;
    gameOver = false;
    gameOverChoice = 0;
    battleMsgTimer = 0;
    flashOverlay = 0;
    flashColor = 0;
    enemyAtkCooldown = currentEnemy().atkSpeed;
    playerAtkCooldown = 0;
    playerAnim = 0;
    enemyAnim = 0;
    lastAnimTime = millis();
    playerX = 120;
    enemyX = 340;
    playerSwordAngle = 0;
    enemySwordAngle = 0;
}

// ── Draw ──
static void drawDojo() {
    // Background (fillRect instead of fillScreen to avoid flicker)
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BG);
    // Floor
    tft.fillRect(0, 210, SCREEN_W, 110, C_FLOOR);
    for (int x = 0; x < SCREEN_W; x += 32) {
        tft.drawFastVLine(x, 210, 110, 0x528A);
    }
    // Horizon line
    tft.drawFastHLine(0, 210, SCREEN_W, TFT_BLACK);
}

static void drawSamurai(int x, int y, bool isPlayer, uint16_t color1, uint16_t color2, int anim, float swordAngle) {
    // Body
    tft.fillRect(x - 12, y - 20, 24, 40, color1);
    // Chest armor
    tft.fillRect(x - 10, y - 14, 20, 18, color2);
    // Head
    tft.fillCircle(x, y - 28, 14, TFT_WHITE);
    // Hair/helmet
    if (isPlayer) {
        tft.fillCircle(x, y - 34, 12, 0x001F); // blue helmet
        tft.fillRect(x - 10, y - 38, 20, 8, 0x001F);
        // Visor
        tft.fillRect(x - 8, y - 34, 16, 4, TFT_WHITE);
    } else {
        tft.fillCircle(x, y - 34, 12, color1);
        tft.fillRect(x - 10, y - 38, 20, 8, color1);
    }
    // Eyes
    tft.fillCircle(x - 4, y - 30, 2, TFT_BLACK);
    tft.fillCircle(x + 4, y - 30, 2, TFT_BLACK);

    // Sword (animated angle)
    int sx = isPlayer ? x + 14 : x - 14;
    int sy = y - 8;
    float sa = swordAngle;
    int ex = sx + (int)(50 * cos(sa));
    int ey = sy + (int)(50 * sin(sa));
    tft.drawLine(sx, sy, ex, ey, TFT_WHITE);
    // Sword guard
    tft.fillRect(sx - 3, sy - 2, 6, 4, 0xFD20);
    // Sword handle
    tft.drawLine(sx, sy, sx - (isPlayer ? -8 : 8), sy + 10, 0x8C44);

    // Legs
    if (anim == 0) {
        tft.fillRect(x - 10, y + 20, 8, 10, C_FLOOR);
        tft.fillRect(x + 2, y + 20, 8, 10, C_FLOOR);
    } else {
        tft.fillRect(x - 12, y + 20, 8, 10, C_FLOOR);
        tft.fillRect(x + 4, y + 20, 8, 10, C_FLOOR);
    }
    // Feet
    tft.fillRect(x - 12, y + 28, 10, 6, isPlayer ? 0x001F : color1);
    tft.fillRect(x + 2, y + 28, 10, 6, isPlayer ? 0x001F : color1);
}

static void playerHitFlash() {
    flashOverlay = 3;     // show for 3 frames
    flashColor = 0xF800;  // red
}

static void enemyHitFlash() {
    flashOverlay = 3;
    flashColor = TFT_YELLOW;
}

static void drawHealthBar(int x, int y, int w, int h, int cur, int max, uint16_t color, uint16_t bg) {
    tft.drawRect(x - 1, y - 1, w + 2, h + 2, TFT_WHITE);
    tft.fillRect(x, y, w, h, bg);
    float f = (float)cur / (float)max;
    if (f < 0) f = 0;
    tft.fillRect(x, y, (int)(w * f), h, color);
    // HP text
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_BG);
    tft.setCursor(x + w / 2 - 12, y + 2);
    tft.print(cur);
    tft.print("/");
    tft.print(max);
}

static void drawEnergyBar(int x, int y, int w, int h) {
    tft.drawRect(x - 1, y - 1, w + 2, h + 2, C_ENERGY_BAR);
    tft.fillRect(x, y, w, h, C_ENERGY_BG);
    float f = (float)playerEnergy / (float)MAX_ENERGY;
    if (f < 0) f = 0;
    tft.fillRect(x, y, (int)(w * f), h, C_ENERGY_BAR);
    tft.setTextSize(1);
    tft.setTextColor(C_ENERGY_BAR, C_BG);
    tft.setCursor(x + 4, y + 2);
    tft.print("ENERGY");
}

static void drawHUD() {
    // Enemy name + level at top center
    tft.setTextSize(2);
    tft.setTextColor(currentEnemy().color2, C_BG);
    String title = "LV." + String(level) + " " + String(currentEnemy().name);
    int tw = tft.textWidth(title.c_str());
    tft.setCursor((SCREEN_W - tw) / 2, 6);
    tft.print(title);

    // Player HP (left side, horizontal)
    drawHealthBar(10, 32, 200, 18, playerHP, playerMaxHP, C_HP_BAR, 0x2124);
    // Player Energy (below HP)
    drawEnergyBar(10, 54, 200, 12);

    // Enemy HP (right side, horizontal)
    drawHealthBar(SCREEN_W - 210, 32, 200, 18, enemyHP, currentEnemy().hp, (enemyHP > currentEnemy().hp / 3) ? C_HP_BAR : C_HP_LOW, 0x2124);

    // Controls hint
    tft.setTextSize(1);
    tft.setTextColor(0x8C71, C_BG);
    tft.setCursor(10, SCREEN_H - 14);
    tft.print("L:Slash  R:Heavy  T:Jump  B:Block  SW:Special");
}

static void drawBattleMsg() {
    if (battleMsgTimer > 0) {
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW, C_BG);
        int tw = tft.textWidth(battleMsg.c_str());
        tft.setCursor((SCREEN_W - tw) / 2, 140);
        tft.print(battleMsg);
        battleMsgTimer--;
    }
}

static void drawScreen() {
    drawDojo();
    // Draw player
    int py = 200;
    drawSamurai(playerX, py, true, C_PLAYER, C_PLAYER2, playerAnim, playerSwordAngle);
    // Draw enemy
    int ey = 200;
    drawSamurai(enemyX, ey, false, currentEnemy().color, currentEnemy().color2, enemyAnim, enemySwordAngle);
    drawHUD();
    drawBattleMsg();
    // Flash overlay (no delay - just drawn for a few frames)
    if (flashOverlay > 0) {
        tft.fillRect(0, 0, SCREEN_W, SCREEN_H, flashColor);
        flashOverlay--;
    }
}

// ── Attack functions ──
static void playerAttack(int dmgMin, int dmgMax, int energyCost, int cooldown, const String& msg, float swordSwing) {
    unsigned long now = millis();
    if (now - lastAtkTime < (unsigned long)cooldown) return;
    if (playerEnergy < energyCost) {
        showMessage("NOT ENOUGH ENERGY!");
        return;
    }
    lastAtkTime = now;
    playerEnergy -= energyCost;
    playerAnim = 1 - playerAnim;

    // Calculate damage
    int dmg = random(dmgMin, dmgMax + 1);
    enemyHP -= dmg;
    if (enemyHP < 0) enemyHP = 0;

    // Sword animation
    playerSwordAngle = swordSwing;

    // Effects
    showMessage(msg + "! -" + String(dmg));
    enemyHitFlash();
    Sounds::sfxEnemyHit();

    // Screen shake
    shakeDur = 5;

    // Check win
    if (enemyHP <= 0) {
        level++;
        enemyIdx = enemyLevelIdx(level);
        enemyHP = currentEnemy().hp;
        // Restore some HP and energy on level up
        playerHP = min(playerMaxHP, playerHP + 20);
        playerEnergy = min(MAX_ENERGY, playerEnergy + 30);
        showMessage(String("DEFEATED! LV.") + String(level - 1));
        Sounds::sfxLevelUp();
        // Draw the level-up screen immediately
        drawScreen();
        delay(300);
        // Increase difficulty slightly
        playerMaxHP = MAX_HP + (level - 1) * 5;
        playerHP = min(playerHP, playerMaxHP);
    }
}

static void enemyAttack() {
    unsigned long now = millis();
    if (now - lastEnemyAtkTime < (unsigned long)enemyAtkCooldown) return;
    lastEnemyAtkTime = now;

    const EnemyInfo& e = currentEnemy();

    // Sometimes enemy blocks
    if (random(0, 5) == 0) {
        enemyAnim = 1;
        enemySwordAngle = 3.14;
        showMessage(String(e.name) + " BLOCKS!");
        delay(200);
        enemyAnim = 0;
        enemySwordAngle = 0;
        return;
    }

    // Calculate damage
    int dmg = random(e.dmgMin, e.dmgMax + 1);

    // Check if player is blocking
    if (now < blockUntil) {
        dmg = dmg * 30 / 100;  // block reduces damage by 70%
        if (dmg < 1) dmg = 1;
        showMessage("BLOCKED! -" + String(dmg));
    } else {
        showMessage(String(e.name) + " HITS! -" + String(dmg));
    }

    playerHP -= dmg;
    if (playerHP < 0) playerHP = 0;

    // Enemy attack animation
    enemyAnim = 1 - enemyAnim;
    enemySwordAngle = -2.0;
    playerHitFlash();
    Sounds::sfxPlayerHit();
    shakeDur = 8;

    // Check game over
    if (playerHP <= 0) {
        gameOver = true;
        g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)level);
        NVS::save();
        Sounds::sfxGameOver();
    }
}

// ── Input ──
static void handleInput() {
    if (Input::pressed(Input::LEFT)) {
        playerAttack(6, 12, 0, 350, "SLASH", -1.2);
    } else if (Input::pressed(Input::RIGHT)) {
        playerAttack(12, 20, 10, 600, "HEAVY", -1.8);
    } else if (Input::pressed(Input::TOP)) {
        playerAttack(8, 16, 15, 450, "JUMP", -2.5);
    } else if (Input::pressed(Input::BOTTOM)) {
        unsigned long now = millis();
        if (now - lastAtkTime > 300) {
            blockUntil = now + 400;
            showMessage("BLOCKING...");
            playerSwordAngle = 0.5;
            Sounds::sfxClick();
        }
    } else if (Input::pressed(Input::SW)) {
        playerAttack(20, 35, 40, 800, "SPECIAL", -3.0);
    }
}

// ── Run ──
void run() {
    gameStarted = false;
    gameOver = false;
    gameOverChoice = 0;
    level = 1;
    enemyIdx = enemyLevelIdx(1);
    playerHP = MAX_HP;
    playerMaxHP = MAX_HP;
    playerEnergy = MAX_ENERGY;
    enemyHP = currentEnemy().hp;
    lastEnergyTime = millis();
    battleMsgTimer = 0;

    tft.fillScreen(C_BG);

    // Title screen
    Display::drawPanel(30, 20, SCREEN_W - 60, 280, 0x0010, TFT_WHITE, 12);

    // Draw samurai silhouette on title
    tft.fillCircle(200, 90, 18, TFT_WHITE);
    tft.fillRect(180, 110, 40, 50, TFT_WHITE);
    tft.drawLine(220, 120, 270, 80, TFT_WHITE);
    // Enemy silhouette
    tft.fillCircle(300, 90, 18, TFT_RED);
    tft.fillRect(280, 110, 40, 50, TFT_RED);
    tft.drawLine(280, 120, 230, 80, TFT_RED);

    // VS text
    tft.setTextSize(3);
    tft.setTextColor(TFT_YELLOW, 0x0010);
    tft.setCursor(220, 70);
    tft.print("VS");

    Display::drawCentredText("SAMURAI FIGHT", 175, 3, TFT_WHITE);
    Display::drawCentredText("Defeat all enemies!", 210, 2, TFT_YELLOW);
    Display::drawCentredText("L:Slash  R:Heavy  T:Jump  B:Block  SW:Special", 245, 1, TFT_LIGHTGREY);
    Display::drawCentredText("Press SW to start", 275, 1, TFT_LIGHTGREY);
    Sounds::sfxSelect();

    while (true) {
        Input::update();
        if (Input::pressed(Input::SW)) {
            Input::beep(25); delay(150);
            gameStarted = true;
            reset();
            break;
        }
        delay(16);
    }

    // ── Game loop ──
    while (true) {
        Input::update();

        if (!gameOver) {
            // Exit with hold BOTTOM + SW
            if (Input::pressed(Input::SW) && Input::held(Input::BOTTOM)) {
                Sounds::sfxBack();
                return;
            }

            // Energy regen
            if (millis() - lastEnergyTime > 50) {
                lastEnergyTime = millis();
                playerEnergy = min(MAX_ENERGY, playerEnergy + 1);
            }

            handleInput();
            enemyAttack();

            // Update display
            drawScreen();

            // Reset sword angles slowly
            if (playerSwordAngle > 0.1) playerSwordAngle -= 0.05;
            else if (playerSwordAngle < -0.1) playerSwordAngle += 0.05;
            else playerSwordAngle = 0;
            if (enemySwordAngle > 0.1) enemySwordAngle -= 0.05;
            else if (enemySwordAngle < -0.1) enemySwordAngle += 0.05;
            else enemySwordAngle = 0;
        } else {
            // ── Game Over ──
            tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BG);
            tft.drawRect(0, 0, SCREEN_W, SCREEN_H, TFT_RED);

            tft.setTextSize(3);
            tft.setTextColor(TFT_RED, C_BG);
            tft.setCursor(70, 50);
            tft.print("GAME OVER");

            tft.setTextSize(2);
            tft.setTextColor(TFT_WHITE, C_BG);
            tft.setCursor(150, 100);
            tft.print("Level: ");
            tft.print(level);
            tft.setCursor(150, 125);
            tft.print("Defeated by: ");
            tft.print(currentEnemy().name);

            uint16_t colA = (gameOverChoice == 0) ? TFT_GREEN : TFT_DARKGREY;
            uint16_t colB = (gameOverChoice == 1) ? TFT_GREEN : TFT_DARKGREY;
            tft.setTextSize(2);
            tft.setTextColor(colA, C_BG);
            tft.setCursor(170, 175);
            tft.print("> Try Again");
            tft.setTextColor(colB, C_BG);
            tft.setCursor(170, 210);
            tft.print("  Back to Menu");
            tft.setTextSize(1);
            tft.setTextColor(TFT_LIGHTGREY, C_BG);
            tft.setCursor(110, 260);
            tft.print("LEFT/RIGHT: select  SW: confirm");

            bool chosen = false;
            while (!chosen) {
                Input::update();
                if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                    gameOverChoice = 1 - gameOverChoice;
                    Input::beep(25);
                    // Redraw choices only (not full screen)
                    uint16_t colA2 = (gameOverChoice == 0) ? TFT_GREEN : TFT_DARKGREY;
                    uint16_t colB2 = (gameOverChoice == 1) ? TFT_GREEN : TFT_DARKGREY;
                    tft.setTextSize(2);
                    tft.setTextColor(colA2, C_BG);
                    tft.setCursor(170, 175);
                    tft.print("> Try Again");
                    tft.setTextColor(colB2, C_BG);
                    tft.setCursor(170, 210);
                    tft.print("  Back to Menu");
                }
                if (Input::pressed(Input::SW)) {
                    Input::beep(25); delay(150);
                    chosen = true;
                }
                if (Input::pressed(Input::TOP)) {
                    delay(150);
                    return;
                }
                delay(16);
            }

            if (gameOverChoice == 0) {
                reset();
            } else {
                return;
            }
        }

        delay(16);
    }
}

} // namespace SamuraiFight

#endif
