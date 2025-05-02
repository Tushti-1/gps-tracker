/*#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Wi-Fi credentials
const char* ssid = "BAPI";
const char* password = "BAPI0509";

// Server URL
const char* server_url = "https://gps-tracker-m2m7.onrender.com/gps";
const char* car_id = "car1";

// GPS setup
TinyGPSPlus gps;
HardwareSerial neogps(2); // RX=16, TX=17 for NEO-6M

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC time

// Motor driver pins
#define IN1 14   // Left motor forward
#define IN2 27   // Left motor reverse
#define IN3 26   // Right motor forward
#define IN4 25   // Right motor reverse
#define ENA 32   // Left motor enable
#define ENB 33   // Right motor enable

#define LED_BUILTIN 2 // ⭐ New: Define LED pin (ESP32 built-in LED)

// Move car forward
void moveForward() {
  analogWrite(ENA, 225);
  analogWrite(ENB, 225);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Stop car
void stopCar() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
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

void setup() {
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, 16, 17); // GPS serial

  // Motor pin modes
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT); // ⭐ New: Set LED as output

  stopCar(); // Safety stop at start

  connectWiFi(); // Connect to Wi-Fi
  timeClient.begin();
  timeClient.update();

  moveForward(); // Start moving by default
  Serial.println("🚗 Car started moving by default!");
}

void loop() {
  // Read GPS data
  while (neogps.available()) {
    gps.encode(neogps.read());
  }

  if (gps.location.isUpdated()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();

    Serial.printf("📍 Latitude: %.6f, Longitude: %.6f\n", lat, lon);
    sendToServer(lat, lon);
  } else {
    Serial.println("Waiting for GPS fix...");
  }

  delay(5000); // Delay between checks
}

String getTimestamp() {
  timeClient.update();
  unsigned long epoch = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epoch);

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

    StaticJsonDocument<200> doc;
    doc["car"] = "car1"; // Specify car ID
    doc["latitude"] = lat;
    doc["longitude"] = lon;
    doc["timestamp"] = getTimestamp();

    String jsonStr;
    serializeJson(doc, jsonStr);

    int httpCode = http.POST(jsonStr);
    String response = http.getString();

    if (httpCode == 201) {
      Serial.println("✅ Data sent successfully");
      Serial.println("📦 Server command: " + response);

      // ⭐ New: Blink LED when data sent successfully
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);

      if (response == "stop") {
        stopCar();
        Serial.println("🛑 Car stopped by server");
      } else if (response == "move") {
        moveForward();
        delay(150);
        stopCar();
        delay(400);
        Serial.println("🚗 Car moving");
      } else {
        Serial.println("⚠️ Unknown command");
      }
    } else {
      Serial.printf("❌ Failed to send data: %d\n", httpCode);
    }

    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected");
    connectWiFi(); // Reconnect if disconnected
  }
}*/





#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Wi-Fi credentials
const char* ssid = "Tushti";
const char* password = "Tushti1234";

// Server URL
const char* server_url = "https://gps-tracker-m2m7.onrender.com/gps";
const char* car_id = "car1";
// GPS setup
TinyGPSPlus gps;
HardwareSerial neogps(2); // RX=16, TX=17 for NEO-6M

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "in.pool.ntp.org", 0, 60000); // UTC time

// Motor driver pins
#define IN1 14   // Left motor forward
#define IN2 27   // Left motor reverse
#define IN3 26   // Right motor forward
#define IN4 25   // Right motor reverse
#define ENA 32   // Left motor enable
#define ENB 33   // Right motor enable

#define LED_BUILTIN 2 // Built-in LED

bool carMoving = false; // 🔥 Tracks whether car is currently moving

// Move car forward
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

// Stop car
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

void setup() {
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, 16, 17); // GPS serial

  // Motor and LED pin modes
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  stopCar(); // Safety stop

  connectWiFi();
  timeClient.begin();
  timeClient.update();
}

void loop() {
  // Read GPS data
  while (neogps.available()) {
    gps.encode(neogps.read());
  }

  if (gps.location.isUpdated()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();

    Serial.printf("📍 Latitude: %.6f, Longitude: %.6f\n", lat, lon);

    // 🚗 Start moving if GPS fix achieved and not already moving
    if (!carMoving) {
      moveForward();
    }

    sendToServer(lat, lon);
  } else {
    Serial.println("Waiting for GPS fix...");
  }

  delay(5000); // Delay between checks
}

String getTimestamp() {
  timeClient.update();
  unsigned long epoch = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epoch);

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

    StaticJsonDocument<200> doc;
    doc["carId"] = "car1"; // Specify car ID
    doc["latitude"] = lat;
    doc["longitude"] = lon;
    doc["timestamp"] = getTimestamp();

    String jsonStr;
    serializeJson(doc, jsonStr);

    int httpCode = http.POST(jsonStr);
    String response = http.getString();

    if (httpCode == 201) {
      Serial.println("✅ Data sent successfully");
      Serial.println("📦 Server command: " + response);

      // Blink LED after successful POST
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);

      if (response == "stop") {
        stopCar();
      } else if (response == "move") {
        if (!carMoving) {
          moveForward();
        }
      } else {
        Serial.println("⚠️ Unknown command received. Ignoring and keep moving...");
        // ⭐ Do nothing: Keep moving even if unknown command received
      }

    } else {
      Serial.printf("❌ Failed to send data: %d\n", httpCode);
    }

    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected");
    connectWiFi(); // Try reconnecting
  }
}

