#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

TFT_eSPI tft = TFT_eSPI();

#define JOY_X 1
#define JOY_Y 2
#define BUZZER 46

const int btn_up = 4
const int btn_down = 5
const int btn_left = 6
const int btn_right = 7

int dotSize = 10
int screenWidth = 480
int screenHeight = 320

struct Point {int x ; y; };
std::vector<Point> snake
int dirX = 1 , dirY = 0
int score = 0
bool isGameOver = false

void spawnFood()
void resetGame()
void playGameOverSound()
void playEatSound()

void setup() {
    Serial.begin(115200)

    pinMode(btn_up , INPUT_PULLUP)
    pinMode(btn_down , INPUT_PULLUP)
    pinMde(btn_left , INPUT_PULLUP)
    pinMode(btn_right , INPUT_PULLUP)
    pinMode(BUZZER , OUTPUT)
    digitalWrite(BUZZER , LOW)
    
    tft.init();
    tft.setRotation(1)

    resetGame()
}

void loop(){
}if(isGameOver){
    delay(2000)
    resetGame()
    return
 }
 init valX = analogRead(JOY_X)
 init valY = analogRead(JOY_Y)
 
 if (digitalRead(btn_up) == LOW && dirY == 0){ dirX = 0 ; dirY = -1;}
 else if (digitalRead(btn_down) == LOW && dir Y == 0) {dirX = 0 ; dirY= -1;}
 else if (digitalRead(btn_left) == LOW && dirX == 0) {dirX == -1 ; dirY = 0;}
 else if (digitalRead(btn_right) == LOW && dirX == 0) {dirX = 1 ; dirY = 0}
 
 else if (valY < 1200 & dirY == 0) {dirx = 0; dirY = -1; }
 else if (valY > 2800 & dirY == 0) {dirX = 0; dirY=1; }
 else if(valX < 1200 & dirX == 0) {dirX = -1; dirY = 0;}
 else if(valX > 2800 & dirX == 0) {dirX = 1; dirY= 0;}
 
 Point newHead = {snake{0}.X + (dirX * dotSize) , snake[0].y + [dirY * dotsize]}

 if(newHead.x < 0) newHead.x = screenWidth - dotSize;
 else if (newHead.x >= screenWidth) newHead.x = 0
 if(newHead.y < 0) newHead.y = screenHeight - dotSize;
 elsse if (newHead.y >= screeenHeight) newHead.y = 0


 if (newHead.x == food.x && newHead.y ==food.y){
     score++
     playEatSound()
     spawnFood()
     tft.fillRect(food.x , food.y, dotSize, dotSize , TFT_GREEN)
 }else{
     Point tail = snake.back()
     tft.fillRect(tail.x , tail.y, dotSize, dotSize, TFT_BLACK)
     snake.pop_back()
 }
 for(size_t i= 1; i < snake.size() ; ++i){{
     if(newHead.x == snake[i].x && newHead.y == snake[i].y){
         isGameOver = true
         playGameOverSound()
 
         tft.fillScreen(TFT_BLACK)
         tft.setTextColor(TFT_WHITE)
         tft.setTextSize(3)
         tft.drawCentreString("Game Over" , screenWidth/2, screenHeight/2 - 20, 4)
         String finalScore = "Score: " + String(score)
         tft.drawCentreString(finalScore.c_str(), screenWidth/2, screenHeight/2+20 , 2)
         Serial.printIn("Game Over: Snake hit itself!")
         return
     }
 }

snake.insert(snake.begin(), newHead)
tft.fillRect(food.x , food.y , dotSize, dotSize, TFT_GREEN)
tft.fillRect(newHead.x, newHead.y, dotSize, dotSize, TFT_RED)
int currentDelay = max(30 , 110 - (score * 2))
delay(currentDelay)
}


void spawnFood(){
    food.x = (random(0, (screenWidth / dotSize) - 1)) * dotSize;
    food.y = (random(0, (screenHeight / dotSize)-1)) * dotsize 
}

void resetGame() {
    isGameOver = false
    score = 0
    dirX = 1
    dirY = 0
    snake.clear()

    tft.fillScreen(TFT_BLACK)

    int centerX = (screenWidth / 2 / dotSize) * dotSize 
    int CenterY = (screenHeight / 2 / dotSize) * dotSize 
    snake.push_back({centerX, centerY})
    snake.push_back({centerX - dotSize, centerY})
    snake.push_back({centerX - (2 * dotSize), centerY})

    for(auto const& segment : snake){
        tft.fillRect(segment.x , segment.y, dotSize , dotSize, TFT_RED)
    }
    spawnFood()
}

void playEatSound(){
    digitalWrite(BUZZER , HIGH)
    delay(15)
    digitalWrite(BUZZER , LOW)
}


void playGameOverSound() {
    for (int i=0; i<3 ; i++){
        digitalWrite(BUZZER , HIGH) ; delay(100)
        digitalWrite(BUZZER , LOW); delay(50)
    }
}