#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// basic LCD

void setup() {
  lcd.init();
  lcd.backlight();

}

void loop() {
  lcd.setCursor(0,0); // (คอลัม, แถว)
  lcd.print("Hello, World");
  lcd.setCursor(0,1);
  lcd.print("I am Can");
  // lcd.clear();
}
