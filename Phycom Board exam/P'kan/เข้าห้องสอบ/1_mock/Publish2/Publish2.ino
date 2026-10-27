#include <WiFiS3.h>
#include <PubSubClient.h>

// ================= WIFI =================
const char* ssid = "Kuncan";
const char* password = "25102548";

// ================= MQTT =================
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;
const char* topic = "68070023/temp"; // ใส่เอาเอง

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
  pinMode(A0, INPUT);
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

  int tempraw = analogRead(A0); // อ่านค่า
  float voltage = (tempraw * 5.0) / 1023.0;
  float temp = (voltage - 0.50) * 100.0;

  char message[20];
  dtostrf(temp, 0, 1, message);
  Serial.println(temp);

  if (client.publish(topic, message)) { // ส่งข้อความขึ้น mqtt
    Serial.print("MQTT Sent: ");
    Serial.println(message);
  } else {
    Serial.println("MQTT Send Failed!");
  }

  delay(1000);
}
