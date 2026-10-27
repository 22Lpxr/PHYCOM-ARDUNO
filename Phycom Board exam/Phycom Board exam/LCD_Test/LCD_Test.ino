/*
  ==============================================================
  Phycom Board Exam - LCD 1602 (I2C) Diagnostic Test
  ==============================================================
  การต่อสายสำหรับจอ LCD แบบมี I2C Backpack:
  - GND  -> GND บน Arduino R4
  - VCC  -> 5V บน Arduino R4
  - SDA  -> ช่อง SDA (หรือ A4) บน Arduino R4
  - SCL  -> ช่อง SCL (หรือ A5) บน Arduino R4
  
  * หากหน้าจอติดแต่ไม่มีตัวหนังสือ ให้ใช้ไขควงหมุน Potentiometer สีฟ้าหลังจอเพื่อปรับ Contrast
  * ต้องติดตั้งไลบรารี LiquidCrystal I2C ใน Arduino IDE (Sketch -> Include Library -> Manage Libraries -> LiquidCrystal I2C by Frank de Brabander)
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ปกติ Address จะเป็น 0x27 หรือ 0x3F (ขนาด 16 ตัวอักษร 2 บรรทัด)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() {
  Serial.begin(9600);
  Serial.println("Starting LCD Test...");

  lcd.init();
  lcd.backlight(); // เปิดไฟ Backlight

  lcd.setCursor(0, 0);
  lcd.print("PHYCOM EXAM TEST");
  lcd.setCursor(0, 1);
  lcd.print("LCD IS READY! :)");

  Serial.println("LCD Test text sent. Check LCD screen!");
}

int count = 0;

void loop() {
  delay(1000);
  count++;
  
  lcd.setCursor(12, 1);
  lcd.print("    "); // ล้างเลขเดิม
  lcd.setCursor(12, 1);
  lcd.print(count);
}
