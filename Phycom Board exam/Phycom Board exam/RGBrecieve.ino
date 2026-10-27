#include "WiFiS3.h"
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi และ Broker
const char* ssid = "2nds";
const char* password = "2Ndsssssss";
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// Topic สำหรับรับข้อความควบคุมสี LED
const char* sub_topic = "68070175/rgb"; 

char mqtt_client_id[40];
WiFiClient espClient;
PubSubClient client(espClient);

// กำหนดขา PWM ของ Arduino Uno R4 ที่ต่อกับ RGB LED
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

// ฟังก์ชันสั่งระดับความสว่างแต่ละสี (0 - 255)
// หมายเหตุ: ถ้าใช้ RGB แบบ Common Cathode (-) ให้ใช้ค่าปกติ 
// แต่ถ้าเป็น Common Anode (+) ให้เปลี่ยนเป็น analogWrite(redPin, 255 - r); แทน
void setColor(int r, int g, int b) {
  analogWrite(redPin, 255 - r);
  analogWrite(greenPin, 255 - g);
  analogWrite(bluePin, 255 - b);
}
// ฟังก์ชันรับข้อความจาก MQTT Server
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // ตัดช่องว่างหรือตัวอักษรขึ้นบรรทัดใหม่ออกเพื่อความแม่นยำ
  message.trim();
  message.toLowerCase(); // แปลงเป็นตัวพิมพ์เล็กทั้งหมดเพื่อให้อ่านง่าย (เช่น Red -> red)

  Serial.print("Received Color Command: ");
  Serial.println(message);

  // --- ครอบคลุมทุกสีหลักและการผสมสีที่เป็นไปได้ ---
  
  if (message == "red") {
    setColor(255, 0, 0);       // สีแดงล้วน
  } 
  else if (message == "green") {
    setColor(0, 255, 0);       // สีเขียวล้วน
  } 
  else if (message == "blue") {
    setColor(0, 0, 255);       // สีน้ำเงินล้วน
  } 
  else if (message == "yellow") {
    setColor(255, 255, 0);     // แดง + เขียว = สีเหลือง
  } 
  else if (message == "cyan") {
    setColor(0, 255, 255);     // เขียว + น้ำเงิน = สีฟ้า/ไซยาน
  } 
  else if (message == "magenta") {
    setColor(255, 0, 255);     // แดง + น้ำเงิน = สีม่วงแดง/มาเจนตา
  } 
  else if (message == "white") {
    setColor(255, 255, 255);   // เปิดครบทุกสี = สีขาว
  } 
  else if (message == "orange") {
    setColor(255, 128, 0);     // การผสมสีส้ม (แดงเยอะ เขียวครึ่งหนึ่ง)
  } 
  else if (message == "purple" || message == "violet") {
    setColor(128, 0, 128);     // สีม่วง
  } 
  else if (message == "off" || message == "black" || message == "0") {
    setColor(0, 0, 0);         // ปิดไฟทั้งหมด
  }
  else {
    Serial.println("Unknown color command!");
  }
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
      Serial.println("Connected & Subscribed to RGB Topic!");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  setColor(0, 0, 0); // เริ่มต้นเคลียร์ให้ไฟดับ

  randomSeed(analogRead(A0)); 
  sprintf(mqtt_client_id, "R4_RGB_%d", random(1000, 9999));

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