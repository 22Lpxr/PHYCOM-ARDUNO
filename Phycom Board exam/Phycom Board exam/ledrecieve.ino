#include "WiFiS3.h"
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi ของห้องสอบ
const char* ssid = "2nds";
const char* password = "2Ndsssssss";

// 2. ตั้งค่า MQTT Broker
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// 3. Topic สำหรับรับค่า (Subscribe)
const char* sub_topic = "68070175/venus";

char mqtt_client_id[40];

WiFiClient espClient;
PubSubClient client(espClient);

// กำหนดขาของ RGB LED (สมมติใช้ขา PWM)
const int redPin = 3;
const int greenPin = 5;
const int bluePin = 6;

// ฟังก์ชันรับข้อความจาก Broker
void callback(char* topic, byte* payload, unsigned int length) {
  // แปลง payload ที่รับมาเป็น String หรือตัวเลข
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  float temp = atof(message); // แปลงข้อความอุณหภูมิเป็นตัวเลขทศนิยม
  Serial.print("Received Temp from Server: ");
  Serial.println(temp);

  // ตรวจสอบเงื่อนไขช่วงอุณหภูมิเพื่อเปลี่ยนสี LED
  if (temp >= 36.0 && temp <= 50.0) {
    // สีแดง
    analogWrite(redPin, 255);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
    Serial.println("LED Color: RED");
  } 
  else if (temp >= 26.0 && temp <= 35.0) {
    // สีฟ้า
    analogWrite(redPin, 0);
    analogWrite(greenPin, 150);
    analogWrite(bluePin, 255);
    Serial.println("LED Color: BLUE");
  } 
  else if (temp >= 10.0 && temp <= 25.0) {
    // สีเขียว
    analogWrite(redPin, 0);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 0);
    Serial.println("LED Color: GREEN");
  }
}

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
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      // ทำการ Subscribe Topic เมื่อเชื่อมต่อสำเร็จ
      client.subscribe(sub_topic);
      Serial.print("Subscribed to Topic: ");
      Serial.println(sub_topic);
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
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  randomSeed(analogRead(A0)); 
  int randomNum = random(1000, 9999);
  sprintf(mqtt_client_id, "R4_Venus_%d", randomNum);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // ผูกฟังก์ชันรับข้อมูล
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}