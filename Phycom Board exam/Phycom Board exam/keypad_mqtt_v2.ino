#include "WiFiS3.h"
#include <PubSubClient.h>

// 1. Wi-Fi
const char* ssid = "2nds";
const char* password = "2Ndsssssss";

// 2. MQTT Broker
const char* mqtt_server = "phycom.it.kmitl.ac.th";
const int mqtt_port = 1883;

// 3. Topic
const char* mqtt_topic = "68070175/keypad";

char mqtt_client_id[40];
WiFiClient espClient;
PubSubClient client(espClient);

// 4. Keypad 4x4 (manual scan, no Keypad library)
const char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
const byte rowPins[4] = {9, 8, 7, 6};
const byte colPins[4] = {5, 4, 3, 2};

const unsigned long DEBOUNCE_MS = 30;

bool lastRaw[4][4];
bool stableState[4][4];
unsigned long lastChange[4][4];

void setup_wifi() {
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
  Serial.print("IP address: ");
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

// Returns the key char on a new, debounced press; otherwise 0
char scanKeypad() {
  char pressed = 0;
  unsigned long now = millis();

  for (byte r = 0; r < 4; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(50);               // let the lines settle

    for (byte c = 0; c < 4; c++) {
      bool raw = (digitalRead(colPins[c]) == LOW);

      if (raw != lastRaw[r][c]) {
        lastRaw[r][c] = raw;
        lastChange[r][c] = now;
      }

      if ((now - lastChange[r][c]) > DEBOUNCE_MS && stableState[r][c] != raw) {
        stableState[r][c] = raw;
        if (raw) pressed = keys[r][c];   // only on press, not release
      }
    }

    pinMode(rowPins[r], INPUT);          // back to high-Z
  }
  return pressed;
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("KEYPAD MQTT v2 running");

  for (byte c = 0; c < 4; c++) pinMode(colPins[c], INPUT_PULLUP);
  for (byte r = 0; r < 4; r++) pinMode(rowPins[r], INPUT);

  randomSeed(analogRead(A0));
  int randomNum = random(1000, 9999);
  sprintf(mqtt_client_id, "R4_Keypad_%d", randomNum);

  Serial.print("Client ID: ");
  Serial.println(mqtt_client_id);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  char key = scanKeypad();
  if (key) {
    Serial.print("Pressed Key: ");
    Serial.println(key);

    char payload[2] = {key, '\0'};
    client.publish(mqtt_topic, payload);
  }
}
