
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//code line 79 รับข้อมูล

// ================= WIFI =================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic = ""; // after

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

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

// ================= RECEIVE MQTT =================
void callback(char* topic, byte* payload,
              unsigned int length) {

  // 1. Convert received payload to String
  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // 2. Convert String to integer
  int value = message.toInt(); // แปลงข้อความเป็น int

  // 3. Display received value on Serial
  Serial.print("Received: ");
  Serial.println(value); // แสดงค่าที่ได้

  // 4. Display received value on LCD
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("MQTT Received:");

  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  lcd.print(value);
}

// ================= CONNECT MQTT =================
void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");

    String clientId = "ArduinoR4-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");

      // Subscribe to receive messages
      client.subscribe(topic); // รับข้อมูลจาก mqtt

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

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  connectWiFi();

  client.setServer(mqtt_server, mqtt_port);

  // Set function for receiving MQTT
  client.setCallback(callback);

  connectMQTT();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQTT Ready!");
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
}
