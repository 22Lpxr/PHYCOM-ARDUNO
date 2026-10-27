
void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
}

// หันด้านหมุนเข้าหาตัว ขาซ้ายเป็น + ขาขวา - ตรงกลาง analogPin

void loop() {
  int potValue = analogRead(A0);
  Serial.print("Potentiometer: ");
  Serial.println(potValue);

  delay(500);
}
