/*
 ===================================================================
                 การต่อขาบน Arduino UNO R4 WiFi
 ===================================================================
 [1] เซ็นเซอร์ Ultrasonic (HC-SR04):
     - VCC   ---> ต่อขา 5V บนบอร์ด
     - GND   ---> ต่อขา GND บนบอร์ด
     - TRIG  ---> ต่อขาดิจิทัล D9
     - ECHO  ---> ต่อขาดิจิทัล D10

 [2] จอแสดงผล LCD 16x2 (I2C Module):
     - VCC   ---> ต่อขา 5V บนบอร์ด
     - GND   ---> ต่อขา GND บนบอร์ด
     - SDA   ---> ต่อขา SDA (อยู่ถัดจาก AREF) หรือขา A4
     - SCL   ---> ต่อขา SCL (อยู่ริมสุดข้าง SDA) หรือขา A5
 ===================================================================
*/

#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ========== 1. ตั้งค่า WiFi & MQTT ==========
const char* ssid        = "YOUR_WIFI_SSID";       // ใส่ชื่อ WiFi
const char* password    = "YOUR_WIFI_PASSWORD";   // ใส่รหัสผ่าน WiFi
const char* mqtt_server = "broker.hivemq.com";    // หรือ Broker ที่อาจารย์กำหนด
const int   mqtt_port   = 1883;

// รหัสนักศึกษา (แก้เป็นของคุณ เช่น "66070123")
const char* student_id  = "YOUR_STUDENT_ID";

// Topic: student_id/taxi_model
char topic[64];

// ========== 2. กำหนดขาอุปกรณ์ ==========
const int TRIG_PIN = 9;   // Ultrasonic TRIG -> D9
const int ECHO_PIN = 10;  // Ultrasonic ECHO -> D10

// ========== 3. ประกาศ Object ==========
// Address จอ I2C ส่วนใหญ่คือ 0x27 (ถ้าจอไม่แสดงผลให้ลอง 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2); 
WiFiClient wifiClient;
PubSubClient client(wifiClient);

String lastModel = "";
unsigned long lastCheckTime = 0;

// ฟังก์ชันเชื่อมต่อ WiFi
void setup_wifi() {
  delay(100);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ฟังก์ชันเชื่อมต่อ MQTT Broker
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "UNOR4-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 3 seconds");
      delay(3000);
    }
  }
}

// ฟังก์ชันวัดระยะทางด้วย Ultrasonic (เซนติเมตร)
float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms (~5 เมตร)
  if (duration == 0) return 999.0; // หากไม่มีวัตถุสะท้อนกลับมา

  return duration * 0.034 / 2.0;
}

void setup() {
  Serial.begin(115200);

  // ตั้งค่าขา Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // ตั้งค่า LCD I2C
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Taxi Meter R4");

  // สร้างชื่อ Topic ตามรูปแบบ: student_id/taxi_model
  sprintf(topic, "%s/taxi_model", student_id);

  // เชื่อมต่อ WiFi และตั้งค่า Broker
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  lcd.clear();
}

void loop() {
  // ตรวจสอบสถานะการเชื่อมต่อ MQTT
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  // อ่านค่าเซนเซอร์ทุกๆ 500 ms
  if (millis() - lastCheckTime >= 500) {
    lastCheckTime = millis();

    float distance = getDistanceCM();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    String currentModel = "";

    // เงื่อนไขจำลองขนาด Leg Room ตามโจทย์
    if (distance < 15.0) {
      currentModel = "Yaris";
    } else if (distance >= 15.0 && distance <= 30.0) {
      currentModel = "Altis";
    } else { // distance > 30.0
      currentModel = "Accord";
    }

    // 1. แสดงผลบน LCD ตามรูปแบบที่กำหนด
    lcd.setCursor(0, 0);
    lcd.print("Taxi Model:     "); // บรรทัดที่ 1: Taxi Model:
    lcd.setCursor(0, 1);
    lcd.print(currentModel + "           "); // บรรทัดที่ 2: รุ่นรถ

    // 2. Publish ไปยัง topic student_id/taxi_model เมื่อรุ่นรถเปลี่ยน
    if (currentModel != lastModel) {
      client.publish(topic, currentModel.c_str());
      Serial.print("Published to [");
      Serial.print(topic);
      Serial.print("]: ");
      Serial.println(currentModel);

      lastModel = currentModel;
    }
  }
}
