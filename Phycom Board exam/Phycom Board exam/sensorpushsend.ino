#include "WiFiS3.h"
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi ของห้องสอบ
const char* ssid = "2nds";
const char* password = "2Ndsssssss";

// 2. ตั้งค่า MQTT Broker
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// 3. กำหนด Topic ให้ตรงกับโจทย์ข้อนี้
const char* mqtt_topic = "68070175/door"; 

// สร้างตัวแปร Client ID แบบสุ่มอัตโนมัติ
char mqtt_client_id[40];

WiFiClient espClient;
PubSubClient client(espClient);

// กำหนดขาอุปกรณ์
const int trigPin = 9;
const int echoPin = 8;
const int buttonPin = 2; // ปุ่มกดต่อกับ Pin 2 และ GND

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // เปิดใช้งานตัวต้านทานภายในสำหรับปุ่ม

  // สุ่ม Client ID ป้องกันการชนกัน
  randomSeed(analogRead(A0)); 
  int randomNum = random(1000, 9999);
  sprintf(mqtt_client_id, "R4_Door_%d", randomNum);
  
  Serial.print("Client ID: ");
  Serial.println(mqtt_client_id);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

unsigned long lastMsg = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // 1. อ่านค่าระยะทางจาก Ultrasonic
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    float distance = duration * 0.034 / 2;

    // 2. อ่านค่าสถานะปุ่มกด (เนื่องจากใช้ INPUT_PULLUP กดแล้วจะเป็น LOW)
    bool isButtonPressed = (digitalRead(buttonPin) == LOW);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm | Button Pressed: ");
    Serial.println(isButtonPressed ? "YES" : "NO");

    // 3. ตรวจสอบเงื่อนไขตามโจทย์
    String payload = "";
    if (distance >= 20.0 && isButtonPressed) {
      payload = "On";
    } else {
      payload = "Off";
    }

    Serial.print("Sending Payload: ");
    Serial.println(payload);

    // Publish ข้อมูลไปยัง Topic
    client.publish(mqtt_topic, payload.c_str());
  }
}