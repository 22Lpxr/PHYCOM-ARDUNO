void setup() {
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
}

void loop() {
  for (int i = 0; i < 255; i++) {
    int light1 = i;

    int light2 = i * 2;
    if (light2 > 255) {
      light2 = 255;
    }

    int light3 = 255 - i;

    int light4 = 128 + (i / 2);

    analogWrite(3, light1);
    analogWrite(5, light2);
    analogWrite(6, light3);
    analogWrite(9, light4);
    delay(10);
  }
  delay(1000);
}
