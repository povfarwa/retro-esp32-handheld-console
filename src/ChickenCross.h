#ifndef CHICKEN_CROSS_H
#define CHICKEN_CROSS_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "config.h"
#include "globals.h"
#include "display.h"
#include "input.h"
#include "sounds.h"
#include "nvs_save.h"

namespace ChickenCross {

// ── Constants ──
static const int COLS       = 10;
static const int VISIBLE    = 9;     // number of visible lanes
static const int LANE_H     = 36;    // pixel height per lane
static const int CHICK_ROW  = 5;     // which screen row chicken stays at
static const int PLAYER_SZ  = 28;

// ── Colors ──
static const uint16_t C_GRASS1    = 0x05E0;
static const uint16_t C_GRASS2    = 0x07E0;
static const uint16_t C_ROAD      = 0x4228;
static const uint16_t C_ROAD_LINE = 0x630C;
static const uint16_t C_WATER     = 0x001F; // blue
static const uint16_t C_WATER2    = 0x061F;
static const uint16_t C_CHICKEN   = 0xFFE0; // yellow
static const uint16_t C_LOG       = 0x8C44; // brown
static const uint16_t C_LOG_TOP   = 0xBE60; // light brown
static const uint16_t C_GATOR     = 0x07E0; // green
static const uint16_t C_GATOR_EYE = 0xFFE0; // yellow eye

static const uint16_t carColors[4] = { 0xF800, 0x001F, 0xFD20, 0xF81F };

// ── Lane types ──
enum LaneType { LANE_GRASS = 0, LANE_ROAD = 1, LANE_RIVER = 2 };

struct Car {
    float x, w;
    int dir, speed;
    uint16_t color;
};

struct Log {
    float x, w;
    int dir, speed;
};

struct Lane {
    LaneType type;
    std::vector<Car> cars;
    std::vector<Log> logs;
};

// ── State ──
static int playerCol = COLS / 2;
static int chickenWorldRow = CHICK_ROW;
static int worldRowStart = 0;
static int score = 0;
static int highScore = 0;
static bool isGameOver = false;
static bool gameStarted = false;
static int gameOverChoice = 0;
static std::vector<Lane> lanes;
static bool ridingLog = false;  // chicken riding a log
static int logCarryX = 0;      // how much log carried chicken
static unsigned long moveCooldown = 0;

// ── Helpers ──
static int screenY(int worldRow) {
    return (worldRow - worldRowStart) * LANE_H;
}

static void addLane(int worldRow) {
    Lane l;
    // Determine type based on worldRow pattern: grass, road, grass, river, repeat
    int pattern = worldRow % 4;
    if (pattern == 0 || pattern == 2) {
        l.type = LANE_GRASS;
    } else if (pattern == 1) {
        l.type = LANE_ROAD;
        int n = random(1, 3);
        for (int i = 0; i < n; i++) {
            Car c;
            c.x  = random(0, SCREEN_W - 56);
            c.w  = 48 + random(0, 16);
            c.dir = random(0, 2) * 2 - 1;
            c.speed = 1 + random(0, 2 + score / 20);
            c.color = carColors[random(0, 4)];
            l.cars.push_back(c);
        }
    } else { // pattern == 3
        l.type = LANE_RIVER;
        int n = 1 + random(0, 2);
        for (int i = 0; i < n; i++) {
            Log lg;
            lg.w  = 52 + random(0, 24);
            lg.x  = random(0, SCREEN_W - (int)lg.w);
            lg.dir = random(0, 2) * 2 - 1;
            lg.speed = 1 + random(0, 2);
            l.logs.push_back(lg);
        }
    }
    lanes.push_back(l);
}

static void scrollUp() {
    worldRowStart++;
    if ((int)lanes.size() > 0) lanes.erase(lanes.begin());
    addLane(worldRowStart + VISIBLE - 1);
}

static void scrollDown() {
    if (worldRowStart <= 0) return;
    worldRowStart--;
    if ((int)lanes.size() > 0) lanes.pop_back();
    // Insert at front
    Lane l;
    int pattern = worldRowStart % 4;
    if (pattern == 0 || pattern == 2) {
        l.type = LANE_GRASS;
    } else if (pattern == 1) {
        l.type = LANE_ROAD;
        int n = random(1, 3);
        for (int i = 0; i < n; i++) {
            Car c;
            c.x  = random(0, SCREEN_W - 56);
            c.w  = 48 + random(0, 16);
            c.dir = random(0, 2) * 2 - 1;
            c.speed = 1 + random(0, 2 + score / 20);
            c.color = carColors[random(0, 4)];
            l.cars.push_back(c);
        }
    } else {
        l.type = LANE_RIVER;
        int n = 1 + random(0, 2);
        for (int i = 0; i < n; i++) {
            Log lg;
            lg.w  = 52 + random(0, 24);
            lg.x  = random(0, SCREEN_W - (int)lg.w);
            lg.dir = random(0, 2) * 2 - 1;
            lg.speed = 1 + random(0, 2);
            l.logs.push_back(lg);
        }
    }
    lanes.insert(lanes.begin(), l);
}

static void reset() {
    playerCol = COLS / 2;
    chickenWorldRow = CHICK_ROW;
    worldRowStart = 0;
    score = 0;
    isGameOver = false;
    ridingLog = false;
    logCarryX = 0;
    gameOverChoice = 0;
    lanes.clear();
    // Build initial visible lanes
    for (int i = 0; i < VISIBLE; i++) {
        addLane(i);
    }
}

// ── Drawing ──
static void drawLane(int idx) {
    if (idx < 0 || idx >= (int)lanes.size()) return;
    int sy = idx * LANE_H;
    Lane& lane = lanes[idx];

    if (lane.type == LANE_GRASS) {
        for (int c = 0; c < COLS; c++) {
            uint16_t g = ((c + idx + worldRowStart) % 2 == 0) ? C_GRASS1 : C_GRASS2;
            tft.fillRect(c * (SCREEN_W / COLS), sy, SCREEN_W / COLS, LANE_H, g);
        }
    } else if (lane.type == LANE_ROAD) {
        tft.fillRect(0, sy, SCREEN_W, LANE_H, C_ROAD);
        // Dashed line
        for (int dx = 0; dx < SCREEN_W; dx += 32) {
            tft.fillRect(dx, sy + LANE_H/2 - 1, 16, 2, C_ROAD_LINE);
        }
        tft.drawFastHLine(0, sy, SCREEN_W, C_ROAD_LINE);
        tft.drawFastHLine(0, sy + LANE_H - 1, SCREEN_W, C_ROAD_LINE);
        // Cars
        for (auto& car : lane.cars) {
            int cx = (int)car.x;
            int cy = sy + 3;
            tft.fillRoundRect(cx, cy, (int)car.w, LANE_H - 6, 4, car.color);
            // windshield
            if (car.dir > 0)
                tft.fillRect(cx + (int)car.w - 14, cy + 4, 10, LANE_H - 14, 0x630C);
            else
                tft.fillRect(cx + 4, cy + 4, 10, LANE_H - 14, 0x630C);
            // wheels
            tft.fillRect(cx + 4, cy, 6, 4, 0x2124);
            tft.fillRect(cx + (int)car.w - 10, cy, 6, 4, 0x2124);
            tft.fillRect(cx + 4, cy + LANE_H - 10, 6, 4, 0x2124);
            tft.fillRect(cx + (int)car.w - 10, cy + LANE_H - 10, 6, 4, 0x2124);
        }
    } else { // RIVER
        // Water with wave pattern
        for (int x = 0; x < SCREEN_W; x += 16) {
            uint16_t wc = ((x / 16 + idx) % 2 == 0) ? C_WATER : C_WATER2;
            tft.fillRect(x, sy, 16, LANE_H, wc);
        }
        // White foam lines
        for (int dx = 0; dx < SCREEN_W; dx += 40) {
            tft.drawFastHLine(dx, sy + 4, 20, TFT_WHITE);
            tft.drawFastHLine(dx + 20, sy + LANE_H - 6, 20, TFT_WHITE);
        }
        // Logs & alligators
        for (auto& log : lane.logs) {
            int lx = (int)log.x;
            int ly = sy + 4;
            int lw = (int)log.w;
            // Log body
            tft.fillRoundRect(lx, ly, lw, LANE_H - 8, 5, C_LOG);
            tft.fillRoundRect(lx + 2, ly + 2, lw - 4, 4, 2, C_LOG_TOP);
            // If small log, it's an alligator
            if (lw < 60) { // alligator
                // Alligator body (green)
                tft.fillRoundRect(lx, ly + 4, lw, LANE_H - 16, 4, C_GATOR);
                // Bumps on back
                for (int b = 0; b < 3; b++) {
                    tft.fillCircle(lx + 8 + b * 10, ly + 4, 3, C_GATOR);
                }
                // Eyes
                tft.fillCircle(lx + 6, ly + 6, 3, TFT_WHITE);
                tft.fillCircle(lx + 6, ly + 6, 1, TFT_BLACK);
                // Snout
                tft.fillTriangle(lx - 4, ly + 8, lx, ly + 4, lx, ly + 12, C_GATOR);
            }
        }
    }
}

static void drawChicken() {
    int idx = chickenWorldRow - worldRowStart;
    if (idx < 0 || idx >= VISIBLE) return;
    int sx = playerCol * (SCREEN_W / COLS) + (SCREEN_W / COLS - PLAYER_SZ) / 2;
    int sy = idx * LANE_H + (LANE_H - PLAYER_SZ) / 2;

    // Body
    tft.fillCircle(sx + PLAYER_SZ/2, sy + PLAYER_SZ/2, PLAYER_SZ/2, C_CHICKEN);
    // Head
    tft.fillCircle(sx + PLAYER_SZ/2, sy + 4, 8, C_CHICKEN);
    // Eyes
    tft.fillCircle(sx + PLAYER_SZ/2 - 3, sy + 3, 2, TFT_BLACK);
    tft.fillCircle(sx + PLAYER_SZ/2 + 3, sy + 3, 2, TFT_BLACK);
    // Beak
    tft.fillTriangle(sx + PLAYER_SZ/2 - 3, sy + 8, sx + PLAYER_SZ/2 + 3, sy + 8, sx + PLAYER_SZ/2, sy + 14, 0xFD20);
    // Comb
    tft.fillCircle(sx + PLAYER_SZ/2, sy - 2, 3, TFT_RED);
    // Feet
    tft.fillRect(sx + 6, sy + PLAYER_SZ - 4, 6, 4, 0xFD20);
    tft.fillRect(sx + PLAYER_SZ - 12, sy + PLAYER_SZ - 4, 6, 4, 0xFD20);
}

static void drawHUD() {
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, C_BLACK);
    tft.setCursor(8, 3);
    tft.print("SCORE: ");
    tft.print(score);
    tft.setCursor(SCREEN_W - 75, 3);
    tft.print("BEST: ");
    tft.print(highScore);
}

// ── Game Over ──
static void drawOverlay() {
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BLACK);
    tft.drawRect(0, 0, SCREEN_W, SCREEN_H, C_CHICKEN);
    tft.setTextSize(3);
    tft.setTextColor(C_CHICKEN, C_BLACK);
    tft.setCursor(60, 48);
    tft.print("GAME OVER");
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, C_BLACK);
    tft.setCursor(150, 100);
    tft.print("Score: "); tft.print(score);
    tft.setCursor(150, 125);
    tft.print("Best: "); tft.print(highScore);

    uint16_t colA = (gameOverChoice == 0) ? C_CHICKEN : TFT_DARKGREY;
    uint16_t colB = (gameOverChoice == 1) ? C_CHICKEN : TFT_DARKGREY;
    tft.setTextSize(2);
    tft.setTextColor(colA, C_BLACK);
    tft.setCursor(170, 175);
    tft.print("> Play Again");
    tft.setTextColor(colB, C_BLACK);
    tft.setCursor(170, 210);
    tft.print("  Back to Menu");
    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, C_BLACK);
    tft.setCursor(110, 260);
    tft.print("LEFT/RIGHT: select  SW: confirm");
}

// ── Collision check ──
static bool isCarHittingChicken(int col, int worldRow) {
    int laneIdx = worldRow - worldRowStart;
    if (laneIdx < 0 || laneIdx >= (int)lanes.size()) return false;
    Lane& lane = lanes[laneIdx];
    if (lane.type != LANE_ROAD) return false;
    int cx = col * (SCREEN_W / COLS) + (SCREEN_W / COLS) / 2;
    for (auto& car : lane.cars) {
        int carCx = (int)car.x + (int)car.w / 2;
        if (abs(cx - carCx) < (SCREEN_W / COLS) / 2 + (int)car.w / 2 - 6) return true;
    }
    return false;
}

static bool isOnLog(int col, int worldRow, float& logDir, int& logSpeed) {
    int laneIdx = worldRow - worldRowStart;
    if (laneIdx < 0 || laneIdx >= (int)lanes.size()) return false;
    Lane& lane = lanes[laneIdx];
    if (lane.type != LANE_RIVER) return false;
    int cx = col * (SCREEN_W / COLS) + 4;
    int cw = (SCREEN_W / COLS) - 8;
    for (auto& log : lane.logs) {
        int logEnd = (int)(log.x + log.w);
        if (cx < logEnd && cx + cw > (int)log.x) {
            logDir = (float)log.dir;
            logSpeed = log.speed;
            return true;
        }
    }
    return false;
}

// ── Game update ──
static void update() {
    if (!gameStarted) { reset(); return; }

    // ── Move cars & logs ──
    for (auto& lane : lanes) {
        if (lane.type == LANE_ROAD) {
            for (auto& car : lane.cars) {
                car.x += car.dir * car.speed;
                if (car.dir > 0 && car.x > SCREEN_W) car.x = -(int)car.w;
                if (car.dir < 0 && car.x + car.w < 0) car.x = SCREEN_W;
            }
        }
        if (lane.type == LANE_RIVER) {
            for (auto& log : lane.logs) {
                log.x += log.dir * log.speed;
                if (log.dir > 0 && log.x > SCREEN_W) log.x = -(int)log.w;
                if (log.dir < 0 && log.x + log.w < 0) log.x = SCREEN_W;
            }
        }
    }

    // ── Move chicken with log if riding ──
    if (ridingLog) {
        int laneIdx = chickenWorldRow - worldRowStart;
        if (laneIdx >= 0 && laneIdx < (int)lanes.size() && lanes[laneIdx].type == LANE_RIVER) {
            for (auto& log : lanes[laneIdx].logs) {
                int logEnd = (int)(log.x + log.w);
                int cx = playerCol * (SCREEN_W / COLS) + 4;
                int cw = (SCREEN_W / COLS) - 8;
                if (cx < logEnd && cx + cw > (int)log.x) {
                    // Move chicken with log
                    float oldX = playerCol * (SCREEN_W / COLS);
                    playerCol = (int)((oldX + log.dir * log.speed) / (SCREEN_W / COLS));
                    if (playerCol < 0) playerCol = 0;
                    if (playerCol >= COLS) playerCol = COLS - 1;
                    break;
                }
            }
        }
        // Check if still on log
        float dummyDir;
        int dummySpeed;
        if (!isOnLog(playerCol, chickenWorldRow, dummyDir, dummySpeed)) {
            ridingLog = false;
            // Fell in water
            isGameOver = true;
            highScore = max(highScore, score);
            g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
            NVS::save();
            Sounds::sfxGameOver();
            return;
        }
    }

    // ── Redraw everything ──
    // Draw lane backgrounds (this overwrites old content cleanly - no fillScreen flash)
    for (int i = 0; i < VISIBLE && i < (int)lanes.size(); i++) {
        drawLane(i);
    }
    // Draw chicken
    int chickIdx = chickenWorldRow - worldRowStart;
    if (chickIdx >= 0 && chickIdx < VISIBLE) {
        drawChicken();
    }
    // HUD (always on top)
    drawHUD();
}

// ── Handle input ──
static void handleInput() {
    if (millis() < moveCooldown) return;

    int newCol = playerCol;
    int newRow = chickenWorldRow;
    bool moved = false;

    if (Input::pressed(Input::LEFT)) {
        newCol = max(0, playerCol - 1);
        moved = true;
    } else if (Input::pressed(Input::RIGHT)) {
        newCol = min(COLS - 1, playerCol + 1);
        moved = true;
    }

    if (Input::pressed(Input::TOP)) {
        newRow = chickenWorldRow + 1;
        moved = true;
    } else if (Input::pressed(Input::BOTTOM)) {
        if (chickenWorldRow > 0) {
            newRow = chickenWorldRow - 1;
            moved = true;
        }
    }

    if (!moved) return;

    moveCooldown = millis() + 180;  // prevent too-fast movement

    // ── Check if new position is safe ──
    // Check cars on road
    if (isCarHittingChicken(newCol, newRow)) {
        isGameOver = true;
        highScore = max(highScore, score);
        g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
        NVS::save();
        Sounds::sfxGameOver();
        return;
    }

    // Check river - must be on log
    int laneIdx = newRow - worldRowStart;
    if (laneIdx >= 0 && laneIdx < (int)lanes.size() && lanes[laneIdx].type == LANE_RIVER) {
        float logDir;
        int logSpeed;
        if (isOnLog(newCol, newRow, logDir, logSpeed)) {
            ridingLog = true;
        } else {
            // Fell in water
            isGameOver = true;
            highScore = max(highScore, score);
            g_app.highScores[2] = max(g_app.highScores[2], (uint32_t)score);
            NVS::save();
            Sounds::sfxGameOver();
            return;
        }
    } else {
        ridingLog = false;
    }

    // ── Apply movement ──
    playerCol = newCol;

    // Track if we moved forward (for score)
    bool movedUp = (newRow > chickenWorldRow);

    chickenWorldRow = newRow;

    // ── Camera scroll ──
    while (chickenWorldRow - worldRowStart > CHICK_ROW) {
        scrollUp();
    }
    while (chickenWorldRow - worldRowStart < 0) {
        scrollDown();
    }

    // ── Score ──
    if (movedUp) {
        score++;
        Sounds::sfxClick();
        if (score > 0 && score % 10 == 0) Sounds::sfxLevelUp();
    }
}

// ── Main run ──
void run() {
    gameStarted = false;
    isGameOver = false;
    gameOverChoice = 0;
    playerCol = COLS / 2;
    chickenWorldRow = CHICK_ROW;
    worldRowStart = 0;
    score = 0;
    ridingLog = false;
    logCarryX = 0;
    moveCooldown = 0;

    tft.fillScreen(C_BLACK);

    // Title screen
    Display::drawPanel(30, 30, SCREEN_W - 60, 260, 0x05E0, C_CHICKEN, 12);

    // Draw chicken on title
    tft.fillCircle(240, 100, 14, C_CHICKEN);
    tft.fillCircle(240, 78, 8, C_CHICKEN);
    tft.fillCircle(237, 77, 2, TFT_BLACK);
    tft.fillCircle(243, 77, 2, TFT_BLACK);
    tft.fillTriangle(238, 82, 242, 82, 240, 88, 0xFD20);

    // Road + water icons on title
    tft.fillRect(50, 210, 60, 12, 0x4228);
    tft.fillRect(130, 210, 60, 8, 0x001F);
    tft.fillRect(250, 210, 60, 12, 0x4228);
    tft.fillRect(370, 210, 60, 8, 0x001F);

    Display::drawCentredText("CHICKEN CROSS", 120, 3, C_CHICKEN);
    Display::drawCentredText("Cross roads & rivers!", 155, 2, TFT_WHITE);
    Display::drawCentredText("ARROWS: move  SW: start", 190, 1, TFT_LIGHTGREY);
    Display::drawCentredText("Press SW to start", 245, 1, TFT_LIGHTGREY);
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

    // ── Main game loop ──
    while (true) {
        Input::update();

        if (!isGameOver) {
            if (Input::pressed(Input::SW)) {
                Sounds::sfxBack();
                return;
            }
            handleInput();
            update();
        } else {
            drawOverlay();
            bool chosen = false;
            while (!chosen) {
                Input::update();
                if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                    gameOverChoice = 1 - gameOverChoice;
                    Input::beep(25);
                    drawOverlay();
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

} // namespace ChickenCross

#endif
