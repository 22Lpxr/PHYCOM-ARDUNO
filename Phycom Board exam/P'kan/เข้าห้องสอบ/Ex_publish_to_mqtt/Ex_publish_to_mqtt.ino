
#include <WiFiS3.h>
#include <PubSubClient.h>
// วิธีส่งค่าขึ้น mqtt

// ================= WIFI =================
const char* ssid = "ืname";
const char* password = "password";

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic = ""; // after

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ================= TEMPERATURE SENSOR =================
const int tempPin = A0;

// ================= CONNECT WIFI =================
void connectWiFi() {
  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
}

// ================= CONNECT MQTT =================
void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");

    String clientId = "ArduinoR4-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(9600);

  // ADC 10-bit (0-1023)
  analogReadResolution(10);

  pinMode(tempPin, INPUT);

  connectWiFi();

  client.setServer(mqtt_server, mqtt_port);
  connectMQTT();
}

// ================= LOOP =================
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  // ==========================================
  // 1. READ TEMPERATURE FROM MCP9700
  // ==========================================

  int tempRaw = analogRead(tempPin);

  // Convert ADC value to voltage
  float voltage = (tempRaw * 5.0) / 1023.0;

  // MCP9700: 500mV at 0C, 10mV per C
  float temperature = (voltage - 0.50) * 100.0;

  // Display temperature on Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  // ==========================================
  // 2. SEND TEMPERATURE TO MQTT
  // ==========================================

  // Convert float temperature to text
  char message[20];
  dtostrf(temperature, 0, 1, message); // แปลงค่าจาก ตัวเลข ไป ข้อความ

  // Publish temperature to MQTT topic
  if (client.publish(topic, message)) { // ส่งข้อความขึ้น mqtt
    Serial.print("MQTT Sent: ");
    Serial.println(message);
  } else {
    Serial.println("MQTT Send Failed!");
  }

  delay(1000);
}
