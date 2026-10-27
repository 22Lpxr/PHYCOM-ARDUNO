/*
  ==============================================================
  Phycom Board Exam - Hardware Diagnostic Suite (Uno R4 WiFi)
  ชุดโค้ดสำหรับตรวจสอบการทำงานของอุปกรณ์ฮาร์ดแวร์ทุกตัวก่อนสอบ
  ==============================================================
  การใช้งาน:
  1. เปิด Serial Monitor ตั้ง Baud Rate เป็น 9600 และเลือก Newline
  2. พิมพ์หมายเลข 1 - 8 แล้วกด Enter เพื่อเลือกโหมดทดสอบอุปกรณ์
  3. พิมพ์ 0 หรือ q แล้วกด Enter เพื่อหยุดการทดสอบและกลับมาหน้าเมนู
*/

#include <Wire.h>

// --- Pin Definitions (ตรงตามโจทย์ข้อสอบ Phycom) ---
// 1. LM35
const int PIN_LM35 = A0;

// 2. Potentiometer
const int PIN_POT = A1;

// 3. Ultrasonic HC-SR04
const int PIN_TRIG = 9;
const int PIN_ECHO = 8;

// 4. Push Button
const int PIN_BTN = 2;

// 5. RGB LED
const int PIN_RGB_R = 3;
const int PIN_RGB_G = 5;
const int PIN_RGB_B = 6;

// 6. 7-Segment (Common Cathode) a, b, c, d, e, f, g
const int segmentPins[] = {2, 3, 4, 5, 6, 7, 8};
const byte digitTable[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

// 7. Keypad 4x4
const byte rowPins[4] = {9, 8, 7, 6};
const byte colPins[4] = {5, 4, 3, 2};
const char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

int activeTest = 0;
unsigned long lastTick = 0;
int segCounter = 0;
int rgbStep = 0;

void printMenu() {
  Serial.println("\n==============================================");
  Serial.println("  [PHYCOM BOARD EXAM] HARDWARE CHECK MENU     ");
  Serial.println("==============================================");
  Serial.println(" 1 : Test เซ็นเซอร์วัดอุณหภูมิ LM35 (A0)");
  Serial.println(" 2 : Test ตัวต้านทานปรับค่าได้ Potentiometer (A1)");
  Serial.println(" 3 : Test เซ็นเซอร์ Ultrasonic HC-SR04 (Trig=9, Echo=8)");
  Serial.println(" 4 : Test ปุ่มกด Push Button (Pin 2)");
  Serial.println(" 5 : Test RGB LED (R=3, G=5, B=6)");
  Serial.println(" 6 : Test จอ 7-Segment (Pin 2 - 8)");
  Serial.println(" 7 : Test สแกน I2C LCD Display (SDA=A4, SCL=A5)");
  Serial.println(" 8 : Test Keypad 4x4 (Row 9-6, Col 5-2)");
  Serial.println(" 0 : Stop / Back to Menu (หรือพิมพ์ q)");
  Serial.println("==============================================");
  Serial.print("เลือกเมนู (1-8): ");
}

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000);
  Wire.begin();
  printMenu();
}

void loop() {
  // รับคำสั่งจาก Serial Monitor
  if (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '\r' || ch == '\n' || ch == ' ') return;

    if (ch >= '1' && ch <= '8') {
      activeTest = ch - '0';
      initCurrentTest();
    } else if (ch == '0' || ch == 'q' || ch == 'Q' || ch == 'm' || ch == 'M') {
      stopAll();
      activeTest = 0;
      printMenu();
    }
  }

  // เรียกการทดสอบตามโหมดที่เลือก
  unsigned long now = millis();
  switch (activeTest) {
    case 1: // LM35
      if (now - lastTick >= 1000) {
        lastTick = now;
        testLM35();
      }
      break;

    case 2: // Potentiometer
      if (now - lastTick >= 300) {
        lastTick = now;
        testPotentiometer();
      }
      break;

    case 3: // Ultrasonic
      if (now - lastTick >= 500) {
        lastTick = now;
        testUltrasonic();
      }
      break;

    case 4: // Push Button
      if (now - lastTick >= 150) {
        lastTick = now;
        testButton();
      }
      break;

    case 5: // RGB LED
      if (now - lastTick >= 1000) {
        lastTick = now;
        testRGBLED();
      }
      break;

    case 6: // 7-Segment
      if (now - lastTick >= 1000) {
        lastTick = now;
        test7Segment();
      }
      break;

    case 7: // I2C Scanner (รันรอบเดียวแล้วรอคำสั่งใหม่)
      testI2CScan();
      activeTest = 0;
      printMenu();
      break;

    case 8: // Keypad 4x4
      testKeypad();
      break;

    default:
      break;
  }
}

// -------------------------------------------------------------
// เริ่มต้นและรีเซ็ตโหมดการทดสอบ
// -------------------------------------------------------------
void initCurrentTest() {
  stopAll();
  lastTick = millis();
  
  if (activeTest == 1) {
    Serial.println("\n[Testing LM35 Temperature Sensor] (ส่ง '0' เพื่อหยุด)");
    Serial.println("ต่อสาย: ขา 1 (ซ้าย)=5V, ขา 2 (กลาง)=A0, ขา 3 (ขวา)=GND (หันด้านแบนเข้าหาตัว)");
  }
  else if (activeTest == 2) {
    Serial.println("\n[Testing Potentiometer] (ส่ง '0' เพื่อหยุด)");
    Serial.println("ต่อสาย: ริมซ้าย=5V, ขากลาง=A1, ริมขวา=GND");
  }
  else if (activeTest == 3) {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    Serial.println("\n[Testing HC-SR04 Ultrasonic] (ส่ง '0' เพื่อหยุด)");
    Serial.println("ต่อสาย: VCC=5V, Trig=Pin 9, Echo=Pin 8, GND=GND");
  }
  else if (activeTest == 4) {
    pinMode(PIN_BTN, INPUT_PULLUP);
    Serial.println("\n[Testing Push Button] (ส่ง '0' เพื่อหยุด)");
    Serial.println("ต่อสาย: ขาหนึ่ง=Pin 2, อีกขาหนึ่ง=GND (ใช้ Internal Pullup ไม่ต้องต่อตัวต้านทาน)");
  }
  else if (activeTest == 5) {
    pinMode(PIN_RGB_R, OUTPUT);
    pinMode(PIN_RGB_G, OUTPUT);
    pinMode(PIN_RGB_B, OUTPUT);
    rgbStep = 0;
    Serial.println("\n[Testing RGB LED] (ส่ง '0' เพื่อหยุด)");
    Serial.println("ต่อสาย: ขายาวสุด=GND, ขา R=Pin 3 (ผ่าน 330 ohm), ขา G=Pin 5, ขา B=Pin 6");
  }
  else if (activeTest == 6) {
    for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);
    segCounter = 0;
    Serial.println("\n[Testing 7-Segment Display] (ส่ง '0' เพื่อหยุด)");
    Serial.println("ต่อสาย Common Cathode: ขา 3 หรือ 8=GND, a..g ต่อกับ Pin 2..8");
  }
  else if (activeTest == 7) {
    Serial.println("\n[Scanning I2C Bus for LCD Display]...");
    Serial.println("ต่อสาย: VCC=5V, GND=GND, SDA=A4 (หรือช่อง SDA), SCL=A5 (หรือช่อง SCL)");
  }
  else if (activeTest == 8) {
    for (byte c = 0; c < 4; c++) pinMode(colPins[c], INPUT_PULLUP);
    for (byte r = 0; r < 4; r++) pinMode(rowPins[r], INPUT);
    Serial.println("\n[Testing Keypad 4x4] (กดปุ่มบน Keypad เพื่อดูผลลัพธ์, ส่ง '0' เพื่อหยุด)");
    Serial.println("ต่อสาย: Rows Pin 1..4 -> Pin 9, 8, 7, 6 | Cols Pin 5..8 -> Pin 5, 4, 3, 2");
  }
}

void stopAll() {
  // ปิด RGB
  analogWrite(PIN_RGB_R, 0);
  analogWrite(PIN_RGB_G, 0);
  analogWrite(PIN_RGB_B, 0);
  // ปิด 7-Segment
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], INPUT);
  }
}

// -------------------------------------------------------------
// 1. ทดสอบ LM35
// -------------------------------------------------------------
void testLM35() {
  int raw = analogRead(PIN_LM35);
  // Uno R4: ADC 10-bit (1023), 5.0V
  float voltage = raw * (5.0 / 1023.0);
  float tempC = voltage * 100.0; // 10mV ต่อ 1 องศาเซลเซียส

  Serial.print("LM35 -> ADC Raw: ");
  Serial.print(raw);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Temp: ");
  Serial.print(tempC, 1);
  Serial.println(" °C");
}

// -------------------------------------------------------------
// 2. ทดสอบ Potentiometer
// -------------------------------------------------------------
void testPotentiometer() {
  int val = analogRead(PIN_POT);
  float volt = val * (5.0 / 1023.0);
  int percent = map(val, 0, 1023, 0, 100);

  Serial.print("Potentiometer (A1) -> Raw: ");
  Serial.print(val);
  Serial.print(" / 1023 | Voltage: ");
  Serial.print(volt, 2);
  Serial.print(" V (");
  Serial.print(percent);
  Serial.println("%)");
}

// -------------------------------------------------------------
// 3. ทดสอบ Ultrasonic HC-SR04
// -------------------------------------------------------------
void testUltrasonic() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // timeout 30ms (~5 เมตร)
  if (duration == 0) {
    Serial.println("Ultrasonic -> [No Echo / Out of range] (ตรวจเช็คสาย Trig Pin 9, Echo Pin 8, ไฟ 5V)");
  } else {
    float distance = duration * 0.0343 / 2.0;
    Serial.print("Ultrasonic -> Distance: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }
}

// -------------------------------------------------------------
// 4. ทดสอบ Push Button
// -------------------------------------------------------------
void testButton() {
  static int lastState = -1;
  int state = digitalRead(PIN_BTN);
  if (state != lastState) {
    lastState = state;
    if (state == LOW) {
      Serial.println("Push Button (Pin 2) -> [ PRESSED! ] (LOW)");
    } else {
      Serial.println("Push Button (Pin 2) -> [ RELEASED ] (HIGH)");
    }
  }
}

// -------------------------------------------------------------
// 5. ทดสอบ RGB LED
// -------------------------------------------------------------
void testRGBLED() {
  rgbStep = (rgbStep + 1) % 5;
  switch (rgbStep) {
    case 1:
      // สีแดง
      analogWrite(PIN_RGB_R, 255);
      analogWrite(PIN_RGB_G, 0);
      analogWrite(PIN_RGB_B, 0);
      Serial.println("RGB LED -> [ RED ] (Pin 3 ON)");
      break;
    case 2:
      // สีเขียว
      analogWrite(PIN_RGB_R, 0);
      analogWrite(PIN_RGB_G, 255);
      analogWrite(PIN_RGB_B, 0);
      Serial.println("RGB LED -> [ GREEN ] (Pin 5 ON)");
      break;
    case 3:
      // สีน้ำเงิน / ฟ้า
      analogWrite(PIN_RGB_R, 0);
      analogWrite(PIN_RGB_G, 0);
      analogWrite(PIN_RGB_B, 255);
      Serial.println("RGB LED -> [ BLUE ] (Pin 6 ON)");
      break;
    case 4:
      // สีขาว (ติดพร้อมกัน 3 สี)
      analogWrite(PIN_RGB_R, 255);
      analogWrite(PIN_RGB_G, 255);
      analogWrite(PIN_RGB_B, 255);
      Serial.println("RGB LED -> [ WHITE ] (R+G+B ON)");
      break;
    case 0:
      // ดับไฟ
      analogWrite(PIN_RGB_R, 0);
      analogWrite(PIN_RGB_G, 0);
      analogWrite(PIN_RGB_B, 0);
      Serial.println("RGB LED -> [ OFF ]");
      break;
  }
}

// -------------------------------------------------------------
// 6. ทดสอบ 7-Segment (Common Cathode)
// -------------------------------------------------------------
void test7Segment() {
  Serial.print("7-Segment -> Showing Number: ");
  Serial.println(segCounter);

  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], digitTable[segCounter][i]);
  }

  segCounter = (segCounter + 1) % 10;
}

// -------------------------------------------------------------
// 7. ทดสอบสแกนหา I2C Address สำหรับจอ LCD
// -------------------------------------------------------------
void testI2CScan() {
  byte count = 0;
  Serial.println("Scanning...");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      if (address == 0x27 || address == 0x3F) {
        Serial.print(" <-- น่าจะเป็นจอ I2C LCD 1602");
      }
      Serial.println();
      count++;
    }
  }
  if (count == 0) {
    Serial.println("No I2C devices found! โปรดตรวจสอบสาย 5V, GND, SDA (A4), SCL (A5)");
  } else {
    Serial.print("Scan complete. พบอุปกรณ์ I2C ทั้งหมด ");
    Serial.print(count);
    Serial.println(" ตัว");
  }
  Serial.println("(อย่าลืมหมุน Potentiometer สีฟ้าหลังจอ LCD เพื่อปรับ Contrast ให้เห็นตัวอักษร)");
}

// -------------------------------------------------------------
// 8. ทดสอบ Keypad 4x4
// -------------------------------------------------------------
void testKeypad() {
  static char lastKey = 0;
  char pressed = 0;

  for (byte r = 0; r < 4; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(50);

    for (byte c = 0; c < 4; c++) {
      if (digitalRead(colPins[c]) == LOW) {
        pressed = keys[r][c];
      }
    }
    pinMode(rowPins[r], INPUT);
  }

  if (pressed != 0 && pressed != lastKey) {
    Serial.print("Keypad Pressed: [ ");
    Serial.print(pressed);
    Serial.println(" ]");
    lastKey = pressed;
    delay(150); // Debounce เล็กน้อย
  } else if (pressed == 0) {
    lastKey = 0;
  }
}
