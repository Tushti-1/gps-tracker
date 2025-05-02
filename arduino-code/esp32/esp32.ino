/*#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// A9G serial connection on UART2 (GPIO 16 = RX, 17 = TX)
HardwareSerial a9g_gps(2);

// WiFi credentials
const char* ssid = "JioFibre";
const char* password = "tushti@1";

// Your local backend server address
//const char* server_url = "http://192.168.45.237:3000/gps";
const char* server_url = "https://gps-tracker-m2m7.onrender.com/gps";
WiFiMulti wifimulti;

// NTP setup for getting timestamp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC time, sync every 60s

void setup() {
  Serial.begin(115200);
  a9g_gps.begin(115200, SERIAL_8N1, 16, 17); // TX, RX to A9G

  // Connect to WiFi
  wifimulti.addAP(ssid, password);
  connectWiFi();

  // Start NTP
  timeClient.begin();
  timeClient.update();

  // Initialize A9G GPS
  delay(2000);
  sendATCommand("AT+RST=1\r\n", 3000); // Reset
  sendATCommand("AT+GPS=1\r\n", 2000); // Enable GPS

  Serial.println("Setup Complete. Tracking started...");
}

void loop() {
  sendATCommand("AT+LOCATION=2\r\n", 5000); // Request GPS location

  if (a9g_gps.available()) {
    String gpsData = a9g_gps.readString();
    Serial.println("📡 Raw GPS Data: " + gpsData);
    parseGPSData(gpsData);
  } else {
    Serial.println("⏳ Waiting for GPS data...");
  }

  delay(5000); // Delay between each request
}

// AT Command Sender
void sendATCommand(const char* command, unsigned long timeout) {
  a9g_gps.println(command);
  delay(timeout);
}

// Parse and send GPS data to backend
void parseGPSData(const String &gpsData) {
  if (gpsData.indexOf("GPS NOT FIX") != -1) {
    Serial.println("No GPS fix yet...");
    return;
  }

  int latStart = gpsData.indexOf("\r\n") + 2;
  int latEnd = gpsData.indexOf(",", latStart);
  int lonStart = latEnd + 1;
  int lonEnd = gpsData.indexOf("\r\n", lonStart);

  if (latStart < 0 || latEnd < 0 || lonStart < 0 || lonEnd < 0) {
    Serial.println("Error parsing GPS data");
    return;
  }

  String latitude = gpsData.substring(latStart, latEnd);
  String longitude = gpsData.substring(lonStart, lonEnd);

  float lat = latitude.toFloat();
  float lon = longitude.toFloat();

  Serial.printf("Latitude: %f, Longitude: %f\n", lat, lon);

  sendToServer(lat, lon);
}

// Send data to Node.js server
void sendToServer(float latitude, float longitude) {
  if (wifimulti.run() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server_url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["latitude"] = latitude;
    jsonDoc["longitude"] = longitude;
    jsonDoc["timestamp"] = getISO8601Timestamp();

    String jsonData;
    serializeJson(jsonDoc, jsonData);

    Serial.print("Sending: ");
    Serial.println(jsonData);

    int responseCode = http.POST(jsonData);
    Serial.print("Server Response Code: ");
    Serial.println(responseCode);

    if (responseCode > 0) {
      String response = http.getString();
      Serial.println("Server replied: " + response);
    } else {
      Serial.println("Failed to send data");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected, trying again...");
    connectWiFi();
  }
}

// Get current UTC timestamp
String getISO8601Timestamp() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  char timestamp[25];
  snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
           ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  return String(timestamp);
}

// Connect to WiFi
void connectWiFi() {
  while (wifimulti.run() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");
}*/












//--------------------------------------------------------------------------------------------------------------------------------------
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

const char* ssid = "Tushti";
const char* password = "Tushti1234";
const char* server_url = "https://gps-tracker-m2m7.onrender.com/gps";

TinyGPSPlus gps;
HardwareSerial neogps(2); // Using Serial2 for NEO-6M

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC time, updates every 60s

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
  neogps.begin(9600, SERIAL_8N1, 16, 17); // RX, TX for NEO-6M

  connectWiFi();
  timeClient.begin();
  timeClient.update();

  Serial.println("GPS tracking started...");
}

void loop() {
  while (neogps.available()) {
    char c = neogps.read();
    gps.encode(c);
  }

  if (gps.location.isUpdated()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();

    Serial.printf("📍 Latitude: %.6f, Longitude: %.6f\n", lat, lon);
    sendToServer(lat, lon);
  } else {
    Serial.println("Waiting for GPS fix...");
  }

  delay(5000); // Delay between GPS checks
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
    doc["latitude"] = lat;
    doc["longitude"] = lon;
    doc["timestamp"] = getTimestamp();

    String jsonStr;
    serializeJson(doc, jsonStr);

    int httpCode = http.POST(jsonStr);
    if (httpCode > 0) {
      Serial.println("✅ Data sent successfully");
    } else {
      Serial.println("❌ Failed to send data");
    }

    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected");
    connectWiFi();
  }
}




//-------------------------------------------------------------------------------------------------
//new code
/*#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

const char* ssid = "Tushti";
const char* password = "Tushti1234";
const char* server_url = "https://gps-tracker-m2m7.onrender.com/gps";

const char* carId = "1"; // 🔁 CHANGE TO "2" FOR SECOND CAR

TinyGPSPlus gps;
HardwareSerial neogps(2);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");
}

void setup() {
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
  connectWiFi();
  timeClient.begin();
  timeClient.update();
}

void loop() {
  while (neogps.available()) {
    char c = neogps.read();
    gps.encode(c);
  }

  if (gps.location.isUpdated()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();
    Serial.printf("📍 Car %s - Lat: %.6f, Lon: %.6f\n", carId, lat, lon);
    sendToServer(lat, lon);
  } else {
    Serial.println("Waiting for GPS fix...");
  }

  delay(5000);
}

String getTimestamp() {
  timeClient.update();
  unsigned long epoch = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epoch);
  char ts[25];
  snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
           ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  return String(ts);
}

void sendToServer(float lat, float lon) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server_url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["carId"] = carId;
    doc["latitude"] = lat;
    doc["longitude"] = lon;
    doc["timestamp"] = getTimestamp();

    String jsonStr;
    serializeJson(doc, jsonStr);
    int httpCode = http.POST(jsonStr);

    if (httpCode > 0) {
      Serial.println("✅ Data sent");
    } else {
      Serial.println("❌ Failed");
    }
    http.end();
  }
}*/


