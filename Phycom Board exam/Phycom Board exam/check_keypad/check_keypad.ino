#include "WiFiS3.h"
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // ต้องติดตั้งไลบรารี LiquidCrystal_I2C ก่อน

// ตั้งค่า I2C LCD (ส่วนใหญ่แอดเดรสจะเป็น 0x27 หรือ 0x3F ขนาด 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 1. ตั้งค่า Wi-Fi และ Broker
const char* ssid = "2nds";
const char* password = "2Ndsssssss";
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// Topic สำหรับรับข้อความมาแสดงบน LCD (ปรับชื่อให้ตรงกับโจทย์หน้างาน)
const char* sub_topic = "68070175/lcd";

char mqtt_client_id[40];
WiFiClient espClient;
PubSubClient client(espClient);

// ฟังก์ชันรับข้อความจาก Server มาแสดงบน LCD
void callback(char* topic, byte* payload, unsigned int length) {
  // แปลง payload เป็น String
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  Serial.print("Received message: ");
  Serial.println(message);

  // แสดงผลบนจอ LCD
  lcd.clear(); // ล้างหน้าจอเก่า
  
  // บรรทัดที่ 1 แสดงชื่อ Topic หรือคำว่า "MQTT Message:"
  lcd.setCursor(0, 0);
  lcd.print("Msg from Server:");
  
  // บรรทัดที่ 2 แสดงข้อความที่ได้รับจาก Server
  lcd.setCursor(0, 1);
  lcd.print(message);
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(mqtt_client_id)) {
      client.subscribe(sub_topic);
      Serial.println("Connected & Subscribed!");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  
  // เริ่มต้นการใช้งานจอ LCD
  lcd.init();                      
  lcd.backlight(); // เปิดไฟพื้นหลังจอ
  lcd.setCursor(0, 0);
  lcd.print("Waiting for MQTT");

  randomSeed(analogRead(A0)); 
  sprintf(mqtt_client_id, "R4_LCD_%d", random(1000, 9999));

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}