
#include <WiFiS3.h>
#include <PubSubClient.h>

// ================= WIFI =================
const char* ssid = "name";
const char* password = "password";

// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic = ""; // ใส่เอาเอง

WiFiClient wifiClient;
PubSubClient client(wifiClient);

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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
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
}
