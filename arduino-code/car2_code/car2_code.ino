#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>

// Wi-Fi credentials
const char* ssid = "Mrsdg";
const char* password = "dg123456";

// Server URL
const char* server_url = "https://gps-tracker-m2m7.onrender.com/gps";
const char* car_id = "car2";

// GPS setup
TinyGPSPlus gps;
HardwareSerial neogps(2); // RX=16, TX=17 for NEO-6M

// NTP setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC time

// Motor driver pins
#define IN1 14
#define IN2 27
#define IN3 26
#define IN4 25
#define ENA 32
#define ENB 33

#define LED_BUILTIN 2

bool carMoving = false;

void moveForward() {
  analogWrite(ENA, 225);
  analogWrite(ENB, 225);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  carMoving = true;
  Serial.println("🚗 Car moving forward");
}

void stopCar() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  carMoving = false;
  Serial.println("🛑 Car stopped");
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to WiFi!");
}

void waitForNTP() {
  Serial.println("Waiting for NTP time...");
  while (!timeClient.update()) {
    Serial.print("⏳ Waiting for NTP time...\n");
    delay(1000);
  }
  Serial.println("✅ NTP time ready!");
}

void setup() {
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, 16, 17);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  stopCar();
  connectWiFi();
  timeClient.begin();
  waitForNTP(); // Wait for proper time
}

void loop() {
  while (neogps.available()) {
    gps.encode(neogps.read());
  }

  if (gps.location.isUpdated()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();
    Serial.printf("📍 Latitude: %.6f, Longitude: %.6f\n", lat, lon);

    if (!carMoving) moveForward();

    sendToServer(lat, lon);
  } else {
    Serial.println("Waiting for GPS fix...");
  }

  delay(5000);
}

String getTimestamp() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  char timestamp[25];
  snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
           ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  return String(timestamp);
}

void sendToServer(float lat, float lon) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server_url);
    http.addHeader("Content-Type", "application/json");

    String timestamp = getTimestamp();

    StaticJsonDocument<256> doc;
    doc["carId"] = car_id;
    doc["latitude"] = lat;
    doc["longitude"] = lon;
    doc["timestamp"] = timestamp;

    String jsonStr;
    serializeJson(doc, jsonStr);

    int httpCode = http.POST(jsonStr);
    String response = http.getString();

    if (httpCode == 201) {
      Serial.println("✅ Data sent successfully");
      Serial.println("📦 Server command: " + response);
      Serial.println("🕒 Timestamp sent: " + timestamp); // ✅ Show timestamp in Serial

      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);

      if (response == "stop") stopCar();
      else if (response == "move" && !carMoving) moveForward();
      else Serial.println("⚠️ Unknown command received. Ignoring and keep moving...");
    } else {
      Serial.printf("❌ Failed to send data: %d\n", httpCode);
    }

    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected");
    connectWiFi();
  }
}
