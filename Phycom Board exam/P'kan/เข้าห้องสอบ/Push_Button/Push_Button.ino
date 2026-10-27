int lastButtonState = HIGH;
void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
}

// LOW = กดปุ่ม
// ขา 2 ข้าง ซ้ายขวา ต่อเข้า Pin ได้เลยแต่ อีกข้างต่องต่อเข้า GND

void loop() {
  int buttonState = digitalRead(2);
  if (buttonState == LOW && lastButtonState == HIGH) {
    Serial.println("Button Pressed!");
    delay(50); // Debounce + ต่อต้านทานด้วย
  }
  lastButtonState = buttonState;
}
