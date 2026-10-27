void setup() {
  pinMode(2, OUTPUT); // 2 ตามขา pin
}

void loop() {
  digitalWrite(2, HIGH); //ติด
  delay(1000);
  digitalWrite(2, LOW); //ดับ
  delay(1000);
}
