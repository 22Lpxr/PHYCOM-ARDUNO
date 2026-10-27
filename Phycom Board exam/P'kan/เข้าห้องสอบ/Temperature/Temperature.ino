void setup() {
  Serial.begin(9600);
}

// วัดอุณหภูมิ
// หันด้านแบนเข้าหาตัว ต่อขาต่างๆ --->  + | Pin | -
void loop() {

  int tempraw = analogRead(A0); // อ่านค่า
  float voltage = (tempraw * 5.0) / 1023.0;
  float temp = (voltage - 0.50) * 100.0;

  Serial.print("Temp: ");
  Serial.print(temp, 1);
  Serial.print(" C");
  Serial.println();
  delay(1000);
}
