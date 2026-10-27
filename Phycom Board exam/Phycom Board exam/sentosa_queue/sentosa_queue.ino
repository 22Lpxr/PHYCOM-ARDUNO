/*
 ===================================================================
                 การต่อขาบน Arduino UNO R4 WiFi
 ===================================================================
 [1] จอแสดงผล LCD 16x2 (I2C):
     - VCC   ---> ขา 5V บนบอร์ด
     - GND   ---> ขา GND บนบอร์ด
     - SDA   ---> ขา SDA (ถัดจาก AREF) หรือขา A4
     - SCL   ---> ขา SCL (ริมสุด) หรือขา A5

 [2] หลอดไฟ Color LED (RGB LED แบบ Common Cathode - ขาลบร่วม):
     - ขา R (แดง)     ---> ขา D3 (PWM)
     - ขา G (เขียว)   ---> ขา D5 (PWM)
     - ขา B (น้ำเงิน) ---> ขา D6 (PWM)
     - ขา GND (ขายาว) ---> ขา GND บนบอร์ด
     *(ถ้าใช้หลอดแบบ Common Anode ให้เปลี่ยน COMMON_ANODE ด้านล่างเป็น true)*
 ===================================================================
*/

#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// หากใช้หลอด RGB แบบ Common Anode (+ ร่วม) ให้เปลี่ยนเป็น true
#define COMMON_ANODE false

// ========== 1. ตั้งค่า WiFi & MQTT ==========
const char* ssid        = "YOUR_WIFI_SSID";       // ใส่ชื่อ WiFi
const char* password    = "YOUR_WIFI_PASSWORD";   // ใส่รหัสผ่าน WiFi
const char* mqtt_server = "broker.hivemq.com";    // หรือ Broker ที่อาจารย์กำหนด
const int   mqtt_port   = 1883;

// รหัสนักศึกษา (แก้เป็นของคุณ เช่น "66070123")
const char* student_id  = "YOUR_STUDENT_ID";

// Topics
char topic_sub_chotiphat[64]; // Universal Studios
char topic_sub_lapas[64];     // S.E.A. Aquarium
char topic_sub_sumet[64];     // Skyline Luge
char topic_pub_choice[64];    // ส่งผลลัพธ์คิวน้อยสุด

// ========== 2. กำหนดขา RGB LED ==========
const int RED_PIN   = 3;
const int GREEN_PIN = 5;
const int BLUE_PIN  = 6;

// ========== 3. ตัวแปรเก็บเวลารอคิว (นาที) ==========
int queue_universal = 0;
int queue_aquarium  = 0;
int queue_luge      = 0;

bool received_universal = false;
bool received_aquarium  = false;
bool received_luge      = false;

String lastPublishedChoice = "";

// ========== 4. ประกาศ Object ==========
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ฟังก์ชันควบคุมสี RGB LED
void setLEDColor(int red, int green, int blue) {
  if (COMMON_ANODE) {
    red   = 255 - red;
    green = 255 - green;
    blue  = 255 - blue;
  }
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

// ฟังก์ชันแสดงสีตามค่าเฉลี่ยคิว
void updateColorLED(float avgQueue) {
  if (avgQueue > 60.0) {
    // 4.a มากกว่า 60 นาที: สีแดง (คนเยอะมาก)
    setLEDColor(255, 0, 0);
  } else if (avgQueue >= 30.0 && avgQueue <= 60.0) {
    // 4.b อยู่ในช่วง 30 - 60 นาที: สีส้ม (คนปานกลาง)
    setLEDColor(255, 80, 0);
  } else {
    // 4.c น้อยกว่า 30 นาที: สีเขียวอมฟ้า (Cyan - คิวโล่ง)
    setLEDColor(0, 255, 180);
  }
}

// ประมวลผลคิว แสดงผล LCD และ Publish ผลลัพธ์
void processQueueData() {
  // 1. หาคิวน้อยที่สุด
  int minTime = queue_universal;
  String bestChoice = "Universal";

  if (queue_aquarium < minTime) {
    minTime = queue_aquarium;
    bestChoice = "Aquarium";
  }
  if (queue_luge < minTime) {
    minTime = queue_luge;
    bestChoice = "Luge";
  }

  // 2. คำนวณค่าเฉลี่ยของทั้ง 3 สถานที่
  float avgQueue = (queue_universal + queue_aquarium + queue_luge) / 3.0;
  int avgDisplay = round(avgQueue);

  // 3. แสดงผลบน LCD
  // บรรทัดที่ 1: เช่น "Luge (15m)"
  char line1[17];
  snprintf(line1, sizeof(line1), "%s (%dm)", bestChoice.c_str(), minTime);
  lcd.setCursor(0, 0);
  lcd.print(line1);
  for (int i = strlen(line1); i < 16; i++) lcd.print(" "); // เคลียร์พื้นที่ว่าง

  // บรรทัดที่ 2: เช่น "Avg: 45m"
  char line2[17];
  snprintf(line2, sizeof(line2), "Avg: %dm", avgDisplay);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  for (int i = strlen(line2); i < 16; i++) lcd.print(" ");

  // 4. แสดงผลสีไฟตามค่าเฉลี่ย
  updateColorLED(avgQueue);

  // 5. Publish ไปยัง student_id/sentosa_choice เมื่อสถานที่เปลี่ยน
  if (bestChoice != lastPublishedChoice) {
    client.publish(topic_pub_choice, bestChoice.c_str());
    Serial.print("Published choice: ");
    Serial.println(bestChoice);
    lastPublishedChoice = bestChoice;
  }
}

// Callback เมื่อได้รับข้อความจาก MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  int queueValue = message.toInt();
  Serial.print("Incoming [");
  Serial.print(topic);
  Serial.print("] -> ");
  Serial.println(queueValue);

  // ตรวจสอบว่ามาจาก topic ของใคร
  if (strstr(topic, "queue_chotiphat") != NULL) {
    queue_universal = queueValue;
    received_universal = true;
  } else if (strstr(topic, "queue_lapas") != NULL) {
    queue_aquarium = queueValue;
    received_aquarium = true;
  } else if (strstr(topic, "queue_sumet") != NULL) {
    queue_luge = queueValue;
    received_luge = true;
  }

  // ประมวลผลและอัปเดตจอ/ไฟ LED ทันทีที่ได้ข้อมูล
  processQueueData();
}

// ฟังก์ชันเชื่อมต่อ WiFi
void setup_wifi() {
  delay(100);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
}

// ฟังก์ชันเชื่อมต่อและ Subscribe MQTT
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "UNOR4-Sentosa-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println(" Connected!");

      // Subscribe ทั้ง 3 topics ของเพื่อน
      client.subscribe(topic_sub_chotiphat);
      client.subscribe(topic_sub_lapas);
      client.subscribe(topic_sub_sumet);

      Serial.println("Subscribed to all 3 queue topics.");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 3 seconds");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // ตั้งค่าขา RGB LED
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setLEDColor(0, 0, 0); // ปิดไฟก่อน

  // ตั้งค่า LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sentosa Queue");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");

  // สร้างชื่อ Topic ตาม student_id
  sprintf(topic_sub_chotiphat, "%s/queue_chotiphat", student_id);
  sprintf(topic_sub_lapas,     "%s/queue_lapas",     student_id);
  sprintf(topic_sub_sumet,     "%s/queue_sumet",     student_id);
  sprintf(topic_pub_choice,    "%s/sentosa_choice",  student_id);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  lcd.clear();
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
}
