#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Wire.h>

int num_array[10][7] = {  { 1,1,1,1,1,1,0 },    // 0
                          { 0,1,1,0,0,0,0 },    // 1
                          { 1,1,0,1,1,0,1 },    // 2
                          { 1,1,1,1,0,0,1 },    // 3
                          { 0,1,1,0,0,1,1 },    // 4
                          { 1,0,1,1,0,1,1 },    // 5
                          { 1,0,1,1,1,1,1 },    // 6
                          { 1,1,1,0,0,0,0 },    // 7
                          { 1,1,1,1,1,1,1 },    // 8
                          { 1,1,1,0,0,1,1 }};   // 9

// Define the pins for each segment of the 7-segment display
const int segment_a = 2; 
const int segment_b = 3; 
const int segment_c = 4;
const int segment_d = 5;
const int segment_e = 6;
const int segment_f = 7;
const int segment_g = 8;

// ================= WIFI =================
const char* ssid = "Kuncan";
const char* password = "25102548";

// ================= MQTT =================
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;
const char* topic = "68070023/prob_stat"; // after

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
}

// ================= RECEIVE MQTT =================
void callback(char* topic, byte* payload, unsigned int length) {

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

  displayDigit(value); // sjow on 7_segment
  delay(1000);
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

void setup() { 
  Serial.begin(9600);
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);

  // Set function for receiving MQTT
  client.setCallback(callback);
  connectMQTT();

  // set pin modes for each segment
  pinMode(segment_a, OUTPUT);   
  pinMode(segment_b, OUTPUT);   
  pinMode(segment_c, OUTPUT);   
  pinMode(segment_d, OUTPUT);   
  pinMode(segment_e, OUTPUT);   
  pinMode(segment_f, OUTPUT);   
  pinMode(segment_g, OUTPUT);   
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();
}

// Function to display a specific digit on the 7-segment display
void displayDigit(int digit) {
  // Set each segment according to the num_array for the given digit
  digitalWrite(segment_a, num_array[digit][0]);
  digitalWrite(segment_b, num_array[digit][1]);
  digitalWrite(segment_c, num_array[digit][2]);
  digitalWrite(segment_d, num_array[digit][3]);
  digitalWrite(segment_e, num_array[digit][4]);
  digitalWrite(segment_f, num_array[digit][5]);
  digitalWrite(segment_g, num_array[digit][6]);
}