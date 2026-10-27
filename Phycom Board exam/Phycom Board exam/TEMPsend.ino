#include "WiFiS3.h"
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi ของห้องสอบ
const char* ssid = "2nds";
const char* password = "2Ndsssssss";

// 2. ตั้งค่า MQTT Broker ตามหน้าเว็บของคุณ
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// 3. กำหนด Topic ให้ตรงกับหน้าเว็บ
const char* mqtt_topic = "68070175/temp"; 

// สร้างตัวแปร Client ID แบบ char array เพื่อให้สามารถสุ่มรหัสใส่ได้
char mqtt_client_id[40];

WiFiClient espClient;
PubSubClient client(espClient);

const int sensorPin = A0; // ขา Analog ของ LM35

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
  
  // สุ่มเลขเพื่อไม่ให้ Client ID ซ้ำกันทุกครั้งที่บอร์ดรีสตาร์ท
  randomSeed(analogRead(A1)); 
  int randomNum = random(1000, 9999);
  sprintf(mqtt_client_id, "R4_68070175_%d", randomNum);
  
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

    // อ่านค่าจาก LM35 (Uno R4 ใช้ไฟ 5V ADC ความละเอียด 10-bit)
    int sensorValue = analogRead(sensorPin);
    float voltage = sensorValue * (5.0 / 1023.0);
    float temperatureC = voltage * 100.0; // สูตร LM35

    String payload = String(temperatureC);

    Serial.print("Sending temperature: ");
    Serial.print(payload);
    Serial.println(" °C");

    client.publish(mqtt_topic, payload.c_str());
  }
}