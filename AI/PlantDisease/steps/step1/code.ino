#include <WiFi.h>
#include <WebServer.h>

// Enter your Wi-Fi credentials
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ESP32 HTTP server
WebServer server(80);

void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods",
                    "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers",
                    "Content-Type");
}

void handleRoot() {
  addCORSHeaders();

  server.send(200, "text/plain",
              "Plant Disease Detection ESP32 Server is running.");
}

void handlePing() {
  addCORSHeaders();

  server.send(200, "application/json",
              "{\"success\":true,\"message\":\"ESP32 is reachable\"}");
}

void handleStatus() {
  addCORSHeaders();

  String json = "{";
  json += "\"success\":true,";
  json += "\"device\":\"ESP32\",";
  json += "\"wifiConnected\":";
  json += (WiFi.status() == WL_CONNECTED) ? "true" : "false";
  json += ",\"ip\":\"";
  json += WiFi.localIP().toString();
  json += "\",\"rssi\":";
  json += String(WiFi.RSSI());
  json += "}";

  server.send(200, "application/json", json);
}

void handleOptions() {
  addCORSHeaders();
  server.send(204);
}

void handleNotFound() {
  addCORSHeaders();
  server.send(404, "application/json",
              "{\"success\":false,\"error\":\"Endpoint not found\"}");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Plant Disease Detection - Step 1");
  Serial.println("Connecting to Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Wi-Fi connected successfully!");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Wi-Fi connection failed.");
    Serial.println("Check Wi-Fi name and password, then restart.");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/ping", HTTP_GET, handlePing);
  server.on("/status", HTTP_GET, handleStatus);

  server.on("/ping", HTTP_OPTIONS, handleOptions);
  server.on("/status", HTTP_OPTIONS, handleOptions);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started on port 80");
  Serial.println("Endpoints: /  /ping  /status");
}

void loop() {
  server.handleClient();

  // Retry Wi-Fi if it disconnects.
  static unsigned long lastReconnectAttempt = 0;

  if (WiFi.status() != WL_CONNECTED &&
      millis() - lastReconnectAttempt > 10000) {
    lastReconnectAttempt = millis();
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}
