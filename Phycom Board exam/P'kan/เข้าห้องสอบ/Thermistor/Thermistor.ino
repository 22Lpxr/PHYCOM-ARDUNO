#include <math.h>
void setup() {
  Serial.begin(9600);
}

// วัดอุณหภูมิเช่นกัน แต่พึ่งตัวต้านทาน
// วิธีต่อตามรูปที่อัพ

void loop() {
  int adc = analogRead(A0);

  float R = 10000.0 * adc / (1023.0 - adc);

  float temp = 1.0 / (log(R / 10000.0) / 3950.0 + 1.0 / 298.15) - 273.15;

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");

  delay(1000);
}