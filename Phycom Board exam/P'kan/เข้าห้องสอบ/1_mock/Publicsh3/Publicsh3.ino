#include <WiFiS3.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= WIFI =================
const char* ssid = "Kuncan";
const char* password = "25102548";

// ================= MQTT =================
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;
const char* topic = "68070023/taxi_model"; // ใส่เอาเอง

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
  pinMode(4, OUTPUT); // TRIG
  pinMode(2, INPUT); // ECHO
  lcd.init();
  lcd.backlight();
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

  digitalWrite(4, HIGH); // เริ่มส่งสัญญาณ
  delayMicroseconds(10);
  digitalWrite(4, LOW); // ปิดส่งสัญญาณ

  int pulseWidth = pulseIn(2, HIGH);
  long distance = pulseWidth / 29 / 2; // ระยะทางจริงๆ
  const char* car;

  if (distance < 15) {
    car = "Yaris";
  } else if (distance <= 30) {
    car = "Altis";
  } else {
    car = "Accord";
  }

  // Send to MQTT
  client.publish(topic, car);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Taxi Model:");

  lcd.setCursor(0, 1);
  lcd.print(car);

  // if (client.publish(topic, message)) { // ส่งข้อความขึ้น mqtt
  //   Serial.print("MQTT Sent: ");
  //   Serial.println(message);
  // } else {
  //   Serial.println("MQTT Send Failed!");
  // }

  delay(1000);
}
