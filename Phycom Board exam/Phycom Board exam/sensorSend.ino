#include "WiFiS3.h"
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi ของห้องสอบ
const char* ssid = "2nds";
const char* password = "2Ndsssssss";

// 2. ตั้งค่า MQTT Broker
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// 3. กำหนด Topic ให้ตรงกับโจทย์ข้อนี้
const char* mqtt_topic = "68070175/food"; 

// สร้างตัวแปร Client ID แบบสุ่มอัตโนมัติเพื่อไม่ให้ชนกับคนอื่น
char mqtt_client_id[40];

WiFiClient espClient;
PubSubClient client(espClient);

// กำหนดขาของ Ultrasonic Sensor
const int trigPin = 9;
const int echoPin = 8;

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

  // สุ่ม Client ID ป้องกันการชนกัน
  randomSeed(analogRead(A0)); 
  int randomNum = random(1000, 9999);
  sprintf(mqtt_client_id, "R4_Ultrasonic_%d", randomNum);
  
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

    // ส่งสัญญาณ Trigger เพื่อวัดระยะทาง
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // อ่านค่าความยาวคลื่นสะท้อนกลับมาเป็นไมโครวินาที
    long duration = pulseIn(echoPin, HIGH);

    // คำนวณแปลงเป็นระยะทางหน่วยเซนติเมตร
    float distance = duration * 0.034 / 2;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // ตรวจสอบเงื่อนไขตามโจทย์
    String payload = "";
    if (distance > 20.0) {
      payload = "off";
    } else {
      payload = "on";
    }

    Serial.print("Sending Payload: ");
    Serial.println(payload);

    // Publish ข้อมูลไปยัง Topic
    client.publish(mqtt_topic, payload.c_str());
  }
}