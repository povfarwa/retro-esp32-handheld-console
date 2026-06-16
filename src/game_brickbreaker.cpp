#include "game_brickbreaker.h"
#include "config.h"
#include "display.h"
#include "input.h"
#include "sounds.h"

namespace BrickBreaker {

static const int HUD_H      = 30;
static const int PLAY_TOP   = HUD_H;
static const int PLAY_W     = SCREEN_W;
static const int PLAY_H     = SCREEN_H - HUD_H;

static const int PAD_W      = 90;
static const int PAD_H      = 10;
static const int PAD_Y      = SCREEN_H - 18;
static const int PAD_SPEED  = 10;

static const int   BALL_R   = 7;
static const float BALL_SPD = 4.0f;

static const int COLS       = 10;
static const int ROWS       = 5;
static const int BRICK_W    = 44;
static const int BRICK_H    = 14;
static const int BRICK_GAP  = 2;
static const int BRICK_OFFX = 4;
static const int BRICK_OFFY = HUD_H + 10;

static const uint16_t ROW_COLORS[ROWS] = {
    TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN
};

static float  ballX, ballY, ballDX, ballDY;
static int    padX, oldPadX;
static bool   bricks[ROWS][COLS];
static int    score, lives;

static void drawHUD() {
    tft.fillRect(0, 0, SCREEN_W, HUD_H, TFT_NAVY);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setCursor(8, 7);
    tft.print("Score: ");
    tft.print(score);

    char livesBuf[20];
    snprintf(livesBuf, sizeof(livesBuf), "Lives: %d", lives);
    int16_t tw = tft.textWidth(livesBuf);
    tft.setCursor(SCREEN_W - tw - 8, 7);
    tft.print(livesBuf);
}

static void drawBrick(int r, int c, bool alive) {
    int x = BRICK_OFFX + c * (BRICK_W + BRICK_GAP);
    int y = BRICK_OFFY + r * (BRICK_H + BRICK_GAP);
    if (alive) {
        tft.fillRect(x, y, BRICK_W, BRICK_H, ROW_COLORS[r]);
        tft.drawRect(x, y, BRICK_W, BRICK_H, TFT_BLACK);
    } else {
        tft.fillRect(x, y, BRICK_W, BRICK_H, TFT_BLACK);
    }
}

static void drawAllBricks() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            drawBrick(r, c, bricks[r][c]);
}

static void drawPaddle(int x, uint16_t color) {
    tft.fillRoundRect(x, PAD_Y, PAD_W, PAD_H, 4, color);
}

static bool anyBrickLeft() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (bricks[r][c]) return true;
    return false;
}

static void showOverlay(const char* title, const char* sub, uint16_t color) {
    Display::drawPanel(60, 100, SCREEN_W - 120, 120, TFT_BLACK, color, 12);
    Display::drawCentredText(title, 118, 3, color);
    Display::drawCentredText(sub,   165, 2, TFT_WHITE);
}

static void initRound() {
    ballX  = SCREEN_W / 2.0f;
    ballY  = SCREEN_H / 2.0f;
    ballDX = BALL_SPD;
    ballDY = -BALL_SPD;
    padX   = (SCREEN_W - PAD_W) / 2;
    oldPadX = padX;
}

static void initGame() {
    score = 0;
    lives = 3;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            bricks[r][c] = true;

    tft.fillScreen(TFT_BLACK);
    drawHUD();
    drawAllBricks();
    initRound();
    drawPaddle(padX, TFT_WHITE);
}

static void waitForSW() {
    while (true) {
        Input::update();
        if (Input::pressed(Input::SW)) { Input::beep(25); delay(150); return; }
        delay(16);
    }
}

void run() {
    tft.fillScreen(TFT_BLACK);
    Display::drawPanel(60, 90, SCREEN_W - 120, 140, TFT_NAVY, TFT_CYAN, 12);
    Display::drawCentredText("BRICK BREAKER", 110, 3, TFT_CYAN);
    Display::drawCentredText("Break all bricks!", 148, 2, TFT_WHITE);
    Display::drawCentredText("Press SW to start", 178, 1, TFT_LIGHTGREY);
    Sounds::sfxSelect();
    waitForSW();

    initGame();

    while (true) {
        Input::update();

        if (Input::pressed(Input::BOTTOM)) {
            Sounds::sfxBack();
            return;
        }

        Input::Axis ax = Input::axis();
        int speed = 0;
        if      (ax.x < -15) speed = -PAD_SPEED;
        else if (ax.x >  15) speed =  PAD_SPEED;
        if (Input::held(Input::LEFT))  speed = -PAD_SPEED;
        if (Input::held(Input::RIGHT)) speed =  PAD_SPEED;

        padX = constrain(padX + speed, 0, SCREEN_W - PAD_W);
        if (padX != oldPadX) {
            drawPaddle(oldPadX, TFT_BLACK);
            drawPaddle(padX, TFT_WHITE);
            oldPadX = padX;
        }

        tft.fillCircle((int)ballX, (int)ballY, BALL_R, TFT_BLACK);
        ballX += ballDX;
        ballY += ballDY;

        if (ballX <= BALL_R)              { ballX = BALL_R;           ballDX = fabsf(ballDX); }
        if (ballX >= SCREEN_W - BALL_R)   { ballX = SCREEN_W - BALL_R; ballDX = -fabsf(ballDX); }
        if (ballY <= PLAY_TOP + BALL_R)   { ballY = PLAY_TOP + BALL_R; ballDY = fabsf(ballDY); }

        if (ballDY > 0 &&
            ballY + BALL_R >= PAD_Y &&
            ballY + BALL_R <= PAD_Y + PAD_H + 4 &&
            ballX >= padX - BALL_R &&
            ballX <= padX + PAD_W + BALL_R)
        {
            ballDY = -fabsf(ballDY);
            float rel = (ballX - (padX + PAD_W / 2.0f)) / (PAD_W / 2.0f);
            ballDX = rel * 6.0f;
            if (fabsf(ballDX) < 1.0f) ballDX = (ballDX >= 0 ? 1.0f : -1.0f);
            Sounds::sfxPaddleHit();
        }

        if (ballY > SCREEN_H + BALL_R) {
            lives--;
            Sounds::sfxLifeLost();
            drawHUD();

            if (lives <= 0) {

                showOverlay("GAME OVER", "", TFT_RED);
                int choice = 0;
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
                    if (Input::pressed(Input::SW)) {
                        Input::beep(25);
                        delay(150);
                        if (choice == 0) { initGame(); break; }
                        else return;
                    }
                    if (Input::pressed(Input::BOTTOM)) {
                        Input::beep(25);
                        delay(150);
                        return;
                    }
                    delay(16);
                }
                continue;
            }
            initRound();
            delay(400);
        }

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (!bricks[r][c]) continue;

                int bx = BRICK_OFFX + c * (BRICK_W + BRICK_GAP);
                int by = BRICK_OFFY + r * (BRICK_H + BRICK_GAP);

                if (ballX + BALL_R > bx && ballX - BALL_R < bx + BRICK_W &&
                    ballY + BALL_R > by && ballY - BALL_R < by + BRICK_H)
                {
                    bricks[r][c] = false;
                    drawBrick(r, c, false);
                    ballDY = -ballDY;
                    score += 10;
                    drawHUD();
                    Sounds::sfxBrickHit();

                    if (!anyBrickLeft()) {
                        Sounds::sfxVictory();
                        showOverlay("YOU WIN!", "", TFT_GREEN);
                        int choice = 0;
                        while (true) {
                            Input::update();
                            if (Input::pressed(Input::LEFT) || Input::pressed(Input::RIGHT)) {
                                choice = 1 - choice;
                                Input::beep(25);
                                if (choice == 0) showOverlay("YOU WIN!", "", TFT_GREEN);
                                else {
                                    tft.fillRect(60, 100, SCREEN_W - 120, 120, TFT_BLACK);
                                    tft.drawRect(60, 100, SCREEN_W - 120, 120, TFT_GREEN);
                                    Display::drawCentredText("YOU WIN!", 118, 3, TFT_GREEN);
                                    Display::drawCentredText("Back to Menu?", 165, 2, TFT_WHITE);
                                }
                            }
                            if (Input::pressed(Input::SW)) {
                                Input::beep(25);
                                delay(150);
                                if (choice == 0) { initGame(); break; }
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
                    goto ball_done;
                }
            }
        }
        ball_done:

        tft.fillCircle((int)ballX, (int)ballY, BALL_R, TFT_YELLOW);

        delay(8);
    }
}

}
