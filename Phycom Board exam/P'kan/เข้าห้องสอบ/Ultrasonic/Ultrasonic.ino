void setup() {
  Serial.begin(9600);
  pinMode(4, OUTPUT); // TRIG
  pinMode(2, INPUT); // ECHO
}

// TRIG ส่งสัญญาณ
// ECHO รับสัญญาณ
// digitalPin

void loop() {
  digitalWrite(4, HIGH); // เริ่มส่งสัญญาณ
  delayMicroseconds(10);
  digitalWrite(4, LOW); // ปิดส่งสัญญาณ

  int pulseWidth = pulseIn(2, HIGH);
  long distance = pulseWidth / 29 / 2; // ระยะทางจริงๆ

  Serial.print("Distance: ");
  Serial.println(distance);
  delay(1000);
}
