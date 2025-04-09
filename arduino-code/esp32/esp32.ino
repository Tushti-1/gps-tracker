#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// A9G serial connection on UART2 (GPIO 16 = RX, 17 = TX)
HardwareSerial a9g_gps(2);

// WiFi credentials
const char* ssid = "BAPI";
const char* password = "BAPI0509";

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
}
