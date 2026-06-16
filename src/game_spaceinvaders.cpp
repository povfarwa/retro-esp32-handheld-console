#include "game_spaceinvaders.h"
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"

namespace SpaceInvaders {

static const int HUD_H       = 30;
static const int PLAY_TOP    = HUD_H;

static const int PLR_W       = 24;
static const int PLR_H       = 14;
static const int PLR_Y       = SCREEN_H - PLR_H - 6;
static const int PLR_SPEED   = 5;

static const int BUL_W       = 3;
static const int BUL_H       = 10;
static const int BUL_SPEED   = 8;
static const int BUL_COOLDOWN= 300;

static const int AL_COLS     = 10;
static const int AL_ROWS     = 4;
static const int AL_W        = 28;
static const int AL_H        = 18;
static const int AL_GAP_X    = 10;
static const int AL_GAP_Y    = 10;
static const int AL_OFFX     = 10;
static const int AL_OFFY     = PLAY_TOP + 10;

static const int MAX_ABUL    = 3;
static const int ABUL_SPEED  = 4;
static const int ABUL_W      = 3;
static const int ABUL_H      = 8;

static int   plrX;
static float bulX, bulY;
static bool  bulActive;
static unsigned long lastShot;

struct AlienBullet { float x, y; bool active; };
static AlienBullet abul[MAX_ABUL];

static bool  aliens[AL_ROWS][AL_COLS];
static float alienOffX;
static int   alienDropY;
static float alienSpeedX;
static bool  alienDir;
static int   aliensLeft;
static int   score;
static int   lives;
static int   level;
static unsigned long lastAlienMove;
static int   alienMoveInterval;

static int aliveInCol(int c) {
    int n = 0;
    for (int r = 0; r < AL_ROWS; r++) if (aliens[r][c]) n++;
    return n;
}

static void drawHUD() {
    tft.fillRect(0, 0, SCREEN_W, HUD_H, TFT_NAVY);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setCursor(8, 7);
    tft.print("Score:"); tft.print(score);

    char buf[24];
    snprintf(buf, sizeof(buf), "Lv:%d  Lives:%d", level, lives);
    int16_t tw = tft.textWidth(buf);
    tft.setCursor(SCREEN_W - tw - 8, 7);
    tft.print(buf);
}

static void drawPlayer(int x, uint16_t color) {
    tft.fillRect(x, PLR_Y + 6, PLR_W, PLR_H - 6, color);
    tft.fillRect(x + PLR_W/2 - 2, PLR_Y, 4, 8, color);
    tft.fillRect(x - 3, PLR_Y + 8, 6, 6, color);
    tft.fillRect(x + PLR_W - 3, PLR_Y + 8, 6, 6, color);
}

static uint16_t alienColor(int row) {
    const uint16_t colors[AL_ROWS] = { TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN };
    return colors[row % AL_ROWS];
}

static void drawAlien(int r, int c, bool alive) {
    int x = AL_OFFX + (int)alienOffX + c * (AL_W + AL_GAP_X);
    int y = AL_OFFY + alienDropY + r * (AL_H + AL_GAP_Y);
    if (alive) {
        tft.fillRect(x, y, AL_W, AL_H, alienColor(r));
        tft.fillRect(x + 4,       y + 4, 5, 5, TFT_BLACK);
        tft.fillRect(x + AL_W-9,  y + 4, 5, 5, TFT_BLACK);
        tft.fillRect(x + 4, y + AL_H-5, AL_W-8, 3, TFT_BLACK);
    } else {
        tft.fillRect(x, y, AL_W, AL_H, TFT_BLACK);
    }
}

static void drawAllAliens() {
    for (int r = 0; r < AL_ROWS; r++)
        for (int c = 0; c < AL_COLS; c++)
            drawAlien(r, c, aliens[r][c]);
}

static void eraseAllAliens() {
    int x = AL_OFFX + (int)alienOffX;
    int y = AL_OFFY + alienDropY;
    tft.fillRect(x - 4, y - 2,
        AL_COLS * (AL_W + AL_GAP_X) + 8,
        AL_ROWS * (AL_H + AL_GAP_Y) + 4,
        TFT_BLACK);
}

static void showOverlay(const char* line1, const char* line2, uint16_t color) {
    Display::drawPanel(60, 100, SCREEN_W - 120, 120, TFT_BLACK, color, 12);
    Display::drawCentredText(line1, 118, 3, color);
    Display::drawCentredText(line2, 168, 2, TFT_WHITE);
}

static void waitForSW() {
    while (true) {
        Input::update();
        if (Input::pressed(Input::SW) || Input::pressed(Input::TOP)) {
            Sounds::sfxClick(); delay(150); return;
        }
        if (Input::pressed(Input::BOTTOM)) {
            Sounds::sfxBack(); return;
        }
        delay(16);
    }
}

static void initLevel() {
    for (int r = 0; r < AL_ROWS; r++)
        for (int c = 0; c < AL_COLS; c++)
            aliens[r][c] = true;
    aliensLeft     = AL_ROWS * AL_COLS;
    alienOffX  = 0;
    alienDropY = 0;
    alienDir   = true;
    alienMoveInterval = max(80, 400 - (level - 1) * 60);
    lastAlienMove  = millis();
    bulActive = false;
    lastShot  = 0;
    for (int i = 0; i < MAX_ABUL; i++) abul[i].active = false;
    plrX = (SCREEN_W - PLR_W) / 2;
    tft.fillScreen(TFT_BLACK);
    drawHUD();
    drawAllAliens();
    drawPlayer(plrX, TFT_CYAN);
}

static void initGame() {
    score = 0;
    lives = 3;
    level = 1;
    initLevel();
}

void run() {
    tft.fillScreen(TFT_BLACK);
    Display::drawPanel(60, 80, SCREEN_W - 120, 160, TFT_NAVY, TFT_GREEN, 12);
    Display::drawCentredText("SPACE INVADERS", 100, 3, TFT_GREEN);
    Display::drawCentredText("Shoot the aliens!", 140, 2, TFT_WHITE);
    Display::drawCentredText("LEFT/RIGHT: move", 168, 1, TFT_LIGHTGREY);
    Display::drawCentredText("TOP: shoot  |  SW: start", 184, 1, TFT_LIGHTGREY);
    Sounds::jingleStartup();
    waitForSW();

    initGame();

    int oldPlrX = plrX;

    while (true) {
        Input::update();

        if (Input::pressed(Input::BOTTOM)) {
            Sounds::sfxBack();
            return;
        }

        Input::Axis ax = Input::axis();
        int spd = 0;
        if (ax.x < -15 || Input::held(Input::LEFT))  spd = -PLR_SPEED;
        if (ax.x >  15 || Input::held(Input::RIGHT))  spd =  PLR_SPEED;
        plrX = constrain(plrX + spd, 0, SCREEN_W - PLR_W);
        if (plrX != oldPlrX) {
            drawPlayer(oldPlrX, TFT_BLACK);
            drawPlayer(plrX, TFT_CYAN);
            oldPlrX = plrX;
        }

        unsigned long now = millis();
        Input::Axis ax2 = Input::axis();
        bool topPressed = Input::pressed(Input::TOP) || ax2.y < -15;
        if (topPressed && !bulActive && now - lastShot > BUL_COOLDOWN && lives > 0) {
            bulActive = true;
            bulX = plrX + PLR_W / 2.0f - BUL_W / 2.0f;
            bulY = PLR_Y - BUL_H;
            lastShot = now;
            Sounds::sfxShoot();
        }

        if (bulActive) {
            tft.fillRect((int)bulX, (int)bulY, BUL_W, BUL_H, TFT_BLACK);
            bulY -= BUL_SPEED;
            if (bulY < PLAY_TOP) {
                bulActive = false;
            } else {
                bool hit = false;
                for (int r = 0; r < AL_ROWS && !hit; r++) {
                    for (int c = 0; c < AL_COLS && !hit; c++) {
                        if (!aliens[r][c]) continue;
                        int ax2 = AL_OFFX + (int)alienOffX + c * (AL_W + AL_GAP_X);
                        int ay  = AL_OFFY + r * (AL_H + AL_GAP_Y);
                        if (bulX + BUL_W > ax2 && bulX < ax2 + AL_W &&
                            bulY < ay + AL_H && bulY + BUL_H > ay)
                        {
                            aliens[r][c] = false;
                            drawAlien(r, c, false);
                            aliensLeft--;
                            score += (AL_ROWS - r) * 10;
                            bulActive = false;
                            hit = true;
                            Sounds::sfxEnemyHit();
                            drawHUD();

                            if (aliensLeft == 0) {
                                Sounds::sfxLevelUp();
                                showOverlay("WAVE CLEAR!", "Press SW to continue", TFT_GREEN);
                                waitForSW();
                                level++;
                                initLevel();
                                oldPlrX = plrX;
                            }
                        }
                    }
                }
                if (bulActive)
                    tft.fillRect((int)bulX, (int)bulY, BUL_W, BUL_H, TFT_WHITE);
            }
        }

        now = millis();
        if (now - lastAlienMove >= (unsigned long)alienMoveInterval) {
            lastAlienMove = now;

            eraseAllAliens();

            int minCol = AL_COLS, maxCol = -1;
            for (int c = 0; c < AL_COLS; c++)
                if (aliveInCol(c) > 0) {
                    if (c < minCol) minCol = c;
                    if (c > maxCol) maxCol = c;
                }

            float step = alienDir ? 6.0f : -6.0f;
            float newOff = alienOffX + step;

            int leftEdge  = AL_OFFX + (int)newOff + minCol * (AL_W + AL_GAP_X);
            int rightEdge = AL_OFFX + (int)newOff + maxCol * (AL_W + AL_GAP_X) + AL_W;

            if (leftEdge <= 0 || rightEdge >= SCREEN_W) {
                alienDir = !alienDir;
                alienDropY += (AL_H / 2 + 4);

                int bottomRow = -1;
                for (int r = AL_ROWS - 1; r >= 0 && bottomRow < 0; r--)
                    for (int c = 0; c < AL_COLS; c++)
                        if (aliens[r][c]) { bottomRow = r; break; }

                if (bottomRow >= 0) {
                    int ay = AL_OFFY + alienDropY + bottomRow * (AL_H + AL_GAP_Y) + AL_H;
                    if (ay >= PLR_Y - 4) {
                        lives = 0;
                    }
                }
            } else {
                alienOffX = newOff;
            }

            drawAllAliens();
        }

        if (lives <= 0) {

            int choice = 0;
            showOverlay("GAME OVER", "", TFT_RED);
            while (true) {
                Input::update();
                if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                    choice = 1 - choice;
                    Input::beep(25);
                    if (choice == 0) showOverlay("GAME OVER", "", TFT_RED);
                    else {
                        tft.fillRect(60, 100, SCREEN_W - 120, 120, TFT_BLACK);
                        tft.drawRect(60, 100, SCREEN_W - 120, 120, TFT_RED);
                        Display::drawCentredText("GAME OVER", 118, 3, TFT_RED);
                        Display::drawCentredText("Back to Menu?", 165, 2, TFT_WHITE);
                    }
                }
                if (Input::pressed(Input::SW) || Input::pressed(Input::TOP)) {
                    Input::beep(25);
                    delay(150);
                    if (choice == 0) { initGame(); oldPlrX = plrX; break; }
                    else return;
                }
                if (Input::pressed(Input::BOTTOM)) {
                    Input::beep(25);
                    delay(150);
                    return;
                }
                delay(16);
            }
        }

        if (random(0, 100) < 3) {
            int col = random(0, AL_COLS);
            for (int r = AL_ROWS - 1; r >= 0; r--) {
                if (aliens[r][col]) {
                    for (int i = 0; i < MAX_ABUL; i++) {
                        if (!abul[i].active) {
                            abul[i].x = AL_OFFX + alienOffX + col * (AL_W + AL_GAP_X) + AL_W / 2;
                            abul[i].y = AL_OFFY + alienDropY + r * (AL_H + AL_GAP_Y) + AL_H;
                            abul[i].active = true;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_ABUL; i++) {
            if (!abul[i].active) continue;
            tft.fillRect((int)abul[i].x, (int)abul[i].y, ABUL_W, ABUL_H, TFT_BLACK);
            abul[i].y += ABUL_SPEED;
            if (abul[i].y > SCREEN_H) {
                abul[i].active = false;
                continue;
            }
            if (abul[i].x + ABUL_W > plrX && abul[i].x < plrX + PLR_W &&
                abul[i].y + ABUL_H > PLR_Y)
            {
                abul[i].active = false;
                lives--;
                Sounds::sfxPlayerHit();
                drawHUD();
                if (lives <= 0) {
                    continue;
                } else {
                    drawPlayer(plrX, TFT_BLACK);
                    delay(300);
                    drawPlayer(plrX, TFT_CYAN);
                }
                continue;
            }
            tft.fillRect((int)abul[i].x, (int)abul[i].y, ABUL_W, ABUL_H, TFT_RED);
        }

        delay(12);
    }
}

}
