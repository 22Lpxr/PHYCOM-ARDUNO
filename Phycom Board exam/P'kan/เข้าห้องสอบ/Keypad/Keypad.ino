
#include <Arduino.h>

// วิธีต่อทางซ้ายเป็น Row ขวาเป็น Col ต่อติดกันเลย
// บรรทัด 85 ตัวแปร stableKey แสดงค่าที่ได้จากการกด
// ================= KEYPAD =================

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

// ================= DEBOUNCE =================
const unsigned long debounceDelay = 50;

char lastRawKey = '\0';
char stableKey = '\0';

unsigned long lastChangeTime = 0;

// ================= SCAN KEYPAD =================
char scanKeypad() {
  char detectedKey = '\0';
  byte pressedCount = 0;

  for (byte c = 0; c < COLS; c++) {
    // Activate one column
    digitalWrite(colPins[c], LOW);
    pinMode(colPins[c], OUTPUT);
    delayMicroseconds(20);

    for (byte r = 0; r < ROWS; r++) {

      if (digitalRead(rowPins[r]) == LOW) {
        detectedKey = keys[r][c];
        pressedCount++;
      }
    }
    // Disable column
    pinMode(colPins[c], INPUT);
  }
  // Accept only one pressed key
  if (pressedCount == 1) {
    return detectedKey;
  }

  return '\0';
}

void setup() {
  Serial.begin(9600);
  // Enable internal pull-up on rows
  for (byte r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  // Columns initially high impedance
  for (byte c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT);
  }
  Serial.println("Keypad Ready!");
}

void loop() {

  char rawKey = scanKeypad();
  // Detect change
  if (rawKey != lastRawKey) {
    lastRawKey = rawKey;
    lastChangeTime = millis();
  }
  // Debounce
  if (millis() - lastChangeTime >= debounceDelay) {
    if (rawKey != stableKey) {
      stableKey = rawKey;
      // Display only on a new press
      if (stableKey != '\0') {
        Serial.print("Key Detected: ");
        Serial.println(stableKey); // **stableKey ตัวแปรแสดงค่าที่ได้จากการกด
      }
    }
  }
}
