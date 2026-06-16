#include <Arduino.h>
#include <SPI.h>
#include <DIYables_TFT_SPI.h>

// ─── Pin definitions ─────────────────────────────────────
#define TFT_BL   21
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
#define TFT_SCK  12
#define TFT_MISO 13
#define TFT_MOSI 11

// ─── Test modes ──────────────────────────────────────────
// Uncomment exactly ONE mode to test:
#define MODE_DEFAULT_SPI      // (1) &SPI - default FSPI bus
// #define MODE_HSPI          // (2) HSPI - independent bus
// #define MODE_DEFAULT_SLOW  // (3) &SPI + 1 MHz init

// ─── Display object ──────────────────────────────────────
#if defined(MODE_HSPI)
  SPIClass mySPI(HSPI);
  DIYables_ILI9488_SPI tft(480, 320, TFT_CS, TFT_DC, TFT_RST, &mySPI);
#else
  DIYables_ILI9488_SPI tft(480, 320, TFT_CS, TFT_DC, TFT_RST);
#endif

// ─── Colors ───────────────────────────────────────────────
#define C_RED    0xF800
#define C_GREEN  0x07E0
#define C_BLUE   0x001F
#define C_WHITE  0xFFFF
#define C_BLACK  0x0000

// ─── Setup ────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n\n=== ILI9488 MINIMAL TEST ===");

  //---- 1. Backlight ON ----
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  Serial.println("BL ON");

  //---- 2. Init SPI bus ----
#if defined(MODE_HSPI)
  Serial.println("Using HSPI");
  mySPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI);
  Serial.println("HSPI begin() done");
#elif defined(MODE_DEFAULT_SLOW)
  Serial.println("Using default SPI, slow init");
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI);
  Serial.println("SPI begin() done");
#else
  Serial.println("Using default SPI");
  // Do NOT call SPI.begin() here — let tft.begin() handle it
#endif

  //---- 3. Init TFT ----
#if defined(MODE_DEFAULT_SLOW)
  tft.begin(1000000);  // 1 MHz init
  Serial.println("tft.begin(1MHz) done");
#else
  tft.begin();         // default 24 MHz
  Serial.println("tft.begin() done");
#endif

  tft.setRotation(1);
  Serial.println("Rotation = 1 (landscape)");

  //---- 4. Color test sequence ----
  Serial.println("FILL RED");
  tft.fillScreen(C_RED);
  delay(3000);

  Serial.println("FILL GREEN");
  tft.fillScreen(C_GREEN);
  delay(3000);

  Serial.println("FILL BLUE");
  tft.fillScreen(C_BLUE);
  delay(3000);

  Serial.println("FILL WHITE");
  tft.fillScreen(C_WHITE);
  delay(3000);

  Serial.println("FILL BLACK");
  tft.fillScreen(C_BLACK);
  delay(3000);

  //---- 5. Test pattern — single pixel ----
  tft.fillScreen(C_BLACK);
  tft.drawPixel(240, 160, C_WHITE);   // center pixel
  tft.drawPixel(241, 160, C_WHITE);
  tft.drawPixel(240, 161, C_WHITE);
  tft.drawPixel(241, 161, C_WHITE);   // 2x2 white dot at center
  Serial.println("Center pixel drawn");

  //---- 6. Text test ----
  tft.setTextColor(C_GREEN, C_BLACK);
  tft.setTextSize(3);
  tft.setCursor(100, 150);
  tft.print("HELLO!");
  tft.setCursor(60, 200);
  tft.setTextSize(2);
  tft.print("ILI9488 480x320");
  Serial.println("Text drawn");
  Serial.println("\n=== TEST COMPLETE ===");
  Serial.println("If you see nothing: check wiring, try a different MODE define");
}

void loop() {
  // Idle
}
