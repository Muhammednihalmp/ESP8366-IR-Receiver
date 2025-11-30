/*******************************************************
 * ESP8266 + IR Receiver + SH1106 OLED
 * STABLE VERSION + SMOOTH ANIMATIONS
 * Creator: Nihal MP
 ********************************************************/

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define IR_RECEIVE_PIN 14      // GPIO14 (D5)
#define OLED_SDA 4             // GPIO4 (D2)
#define OLED_SCL 5             // GPIO5 (D1)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

unsigned long lastDisplayUpdate = 0;

// ================== MATRIX RAIN (Improved) ==================
#define COLS 16
#define ROWS 8
uint8_t drops[COLS];
bool activeCol[COLS];

void initRain() {
  for (int i = 0; i < COLS; i++) {
    drops[i] = random(0, ROWS);
    activeCol[i] = false;
  }
}

void drawMatrixRain() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  for (int x = 0; x < COLS; x++) {
    int colX = x * 8;

    if (!activeCol[x] && random(100) < 8) activeCol[x] = true;

    if (activeCol[x]) {
      char c = random(33, 126);

      display.setCursor(colX, drops[x] * 8);
      display.print(c);

      drops[x]++;

      if ((drops[x] * 8) > 63) {
        drops[x] = 0;
        activeCol[x] = false;
      }
    }
  }

  display.display();
}

// ================== GLITCH TEXT (Improved) ==================
void glitchText(const char* txt) {
  String s = txt;
  for (int i = 0; i < s.length(); i++) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 20);

    for (int j = 0; j <= i; j++)
      display.print(s[j]);

    for (int j = i + 1; j < s.length(); j++) {
      display.print((char)random(33, 126));
    }
    display.display();
    delay(80);
  }
  delay(300);
}

// ================== NEW SMOOTH WAVE ANIMATION ==================
void irWaveAnimation() {
  for (int t = 0; t < 80; t++) {
    display.clearDisplay();

    for (int x = 0; x < 128; x++) {
      int y = 32 + (sin((x + t) * 0.15) * 15);  
      display.drawPixel(x, y, SH110X_WHITE);
    }

    // Center IR dot
    display.fillCircle(64, 32, 5, SH110X_WHITE);

    display.display();
    delay(25);
  }
}

// ================== OLED PRINT (Stable) ==================
void showOLED(String L1, String L2 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(L1);

  if (L2.length()) {
    display.setCursor(0, 16);
    display.println(L2);
  }

  display.display();
}

// ================== BOOT ANIMATION ==================
void bootAnimation() {
  randomSeed(millis());

  initRain();
  for (int i = 0; i < 55; i++) {
    drawMatrixRain();
    delay(60);
  }

  display.clearDisplay();
  display.setCursor(30, 10);
  display.println("DECODING...");
  display.display();
  delay(600);

  glitchText("Nihal MP");

  irWaveAnimation();

  showOLED("IR LOCKED & READY", "by Nihal MP");
  delay(1200);
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(OLED_ADDR, true)) {
    Serial.println("OLED FAILED");
    while (1) delay(100);
  }

  irrecv.enableIRIn();

  bootAnimation();

  showOLED("IR Receiver Ready", "Press any button");
}

// ================== LOOP ==================
void loop() {
  if (irrecv.decode(&results)) {

    if (results.decode_type != UNKNOWN) {  
      String proto = typeToString(results.decode_type);
      String raw = String(results.value, HEX);
      raw.toUpperCase();

      showOLED("Proto: " + proto, "Code: 0x" + raw);
    }

    irrecv.resume();
  }
}
