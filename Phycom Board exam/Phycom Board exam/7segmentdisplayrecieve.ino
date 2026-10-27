#include "WiFiS3.h"
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi ของห้องสอบ
const char* ssid = "2nds";
const char* password = "2Ndsssssss";

// 2. ตั้งค่า MQTT Broker
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// 3. Topic สำหรับรับค่า 7-Segment
const char* sub_topic = "68070175/prob_stat";

char mqtt_client_id[40];

WiFiClient espClient;
PubSubClient client(espClient);

// กำหนดขาของ 7-Segment (a, b, c, d, e, f, g)
const int segmentPins[] = {2, 3, 4, 5, 6, 7, 8};

// ตารางรหัสสำหรับแสดงตัวเลข 0 - 9 บนจอ 7-Segment แบบ Common Cathode (1 = เปิดไฟ, 0 = ปิด)
const byte digitTable[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

void displayNumber(int num) {
  if (num >= 0 && num <= 9) {
    for (int i = 0; i < 7; i++) {
      digitalWrite(segmentPins[i], digitTable[num][i]);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  int receivedNum = atoi(message);
  Serial.print("Received Number for 7-Segment: ");
  Serial.println(receivedNum);

  displayNumber(receivedNum);
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
  
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }

  randomSeed(analogRead(A0)); 
  int randomNum = random(1000, 9999);
  sprintf(mqtt_client_id, "R4_7Seg_%d", randomNum);

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