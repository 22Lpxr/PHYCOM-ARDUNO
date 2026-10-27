
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>
#include <stdio.h>

// ========== WiFi ==========
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ========== MQTT ==========
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

const char* topic1 = "student_id/queue_chotiphat";
const char* topic2 = "student_id/queue_lapas";
const char* topic3 = "student_id/queue_sumet";
const char* topic_out = "student_id/sentosa_choice";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ========== LCD ==========
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ========== RGB Common Anode ==========
const int RED = 9;
const int GREEN = 10;
const int BLUE = 11;

// ========== Queue Data ==========
int universal = 0;
int aquarium = 0;
int luge = 0;

bool gotUniversal = false;
bool gotAquarium = false;
bool gotLuge = false;

// ========== RGB Function ==========
void setColor(int r, int g, int b) {
  // Common Anode: invert PWM
  analogWrite(RED, 255 - r);
  analogWrite(GREEN, 255 - g);
  analogWrite(BLUE, 255 - b);
}

// ========== LCD Function ==========
void showLCD(const char* place, int minTime, int avg) {
  char line[17];

  lcd.clear();

  lcd.setCursor(0, 0);
  snprintf(line, sizeof(line), "%s (%dm)", place, minTime);
  lcd.print(line);

  lcd.setCursor(0, 1);
  snprintf(line, sizeof(line), "Avg: %dm", avg);
  lcd.print(line);
}

// ========== Calculate ==========
void calculateQueue() {

  // ต้องได้รับค่าทั้ง 3 สถานที่ก่อน
  if (!gotUniversal || !gotAquarium || !gotLuge) {
    return;
  }

  float average = (universal + aquarium + luge) / 3.0;
  int avgDisplay = (int)(average + 0.5);

  int minTime = universal;
  const char* place = "Universal";

  if (aquarium < minTime) {
    minTime = aquarium;
    place = "Aquarium";
  }

  if (luge < minTime) {
    minTime = luge;
    place = "Luge";
  }

  // 1. Publish สถานที่คิวน้อยที่สุด
  client.publish(topic_out, place);

  // 2. LCD
  showLCD(place, minTime, avgDisplay);

  // 3. RGB LED จากค่าเฉลี่ยจริง
  if (average > 60) {
    setColor(255, 0, 0);       // Red
  }
  else if (average >= 30) {
    setColor(255, 100, 0);     // Orange
  }
  else {
    setColor(0, 255, 255);     // Cyan
  }

  // Serial Monitor
  Serial.print("Universal: ");
  Serial.println(universal);

  Serial.print("Aquarium: ");
  Serial.println(aquarium);

  Serial.print("Luge: ");
  Serial.println(luge);

  Serial.print("Average: ");
  Serial.println(average);

  Serial.print("Best place: ");
  Serial.println(place);
  Serial.println("-------------------");
}

// ========== MQTT Callback ==========
void callback(char* topic, byte* payload, unsigned int length) {

  // แปลง MQTT Payload เป็นข้อความ
  char message[32];

  if (length == 0 || length >= sizeof(message)) {
    return;
  }

  memcpy(message, payload, length);
  message[length] = '\0';

  // ตรวจสอบว่าเป็นตัวเลขจำนวนเต็มไม่ติดลบ
  char* endptr;
  long number = strtol(message, &endptr, 10);

  if (endptr == message || *endptr != '\0' ||
      number < 0 || number > 10000) {
    Serial.println("Invalid queue value");
    return;
  }

  int queueTime = (int)number;

  if (strcmp(topic, topic1) == 0) {
    universal = queueTime;
    gotUniversal = true;
  }
  else if (strcmp(topic, topic2) == 0) {
    aquarium = queueTime;
    gotAquarium = true;
  }
  else if (strcmp(topic, topic3) == 0) {
    luge = queueTime;
    gotLuge = true;
  }
  else {
    return;
  }

  calculateQueue();
}

// ========== MQTT Reconnect ==========
void reconnect() {
  while (!client.connected()) {

    Serial.print("Connecting MQTT... ");

    // ใช้ Client ID ที่แตกต่างจากอุปกรณ์อื่น
    String clientId = "Sentosa_R4_";
    clientId += String((unsigned long)random(0x7fffffff), HEX);

    if (client.connect(clientId.c_str())) {

      Serial.println("Connected!");

      client.subscribe(topic1);
      client.subscribe(topic2);
      client.subscribe(topic3);

      Serial.println("Subscribed to 3 topics");

    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// ========== Setup ==========
void setup() {
  Serial.begin(9600);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  setColor(0, 0, 0); // ปิด LED

  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sentosa Queue");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting WiFi...");
  }

  Serial.println("WiFi Connected!");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  reconnect();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("3 queues...");
}

// ========== Loop ==========
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}
