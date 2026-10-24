#include <WiFi.h>
#include <WebServer.h>

// ==============================
// Configuration
// ==============================
// Wi-Fi credentials
const char* WIFI_SSID = "GoSpaze_WFD_4F_2.4G";
const char* WIFI_PASSWORD = "9845454344";

// Email settings remain unchanged and are kept in firmware only.
// These are not used by the browser frontend.
const char* SMTP_SENDER_EMAIL = "academytechbots@gmail.com";
const char* SMTP_RECIPIENT_EMAIL = "shaiksameera72859@example.com";
const char* SMTP_APP_PASSWORD = "ukmmjlfkogrtgujs";

// ==============================
// ESP32 WebServer
// ==============================
WebServer server(80);

// Latest plant result received from frontend
String latestPlantName = "Not available";
String latestAssessment = "Not available";
String latestDetails = "No result received yet.";
float latestConfidence = 0.0;

// ==============================
// CORS helper
// ==============================
void setCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, Api-Key");
  server.sendHeader("Access-Control-Allow-Credentials", "false");
}

// ==============================
// Utility functions
// ==============================
String readRequestBody() {
  if (server.hasArg("plain")) {
    return server.arg("plain");
  }
  return "";
}

String extractJsonStringValue(const String& json, const String& key) {
  String searchKey = "\"" + key + "\"";
  int keyIndex = json.indexOf(searchKey);
  if (keyIndex == -1) return "";

  int colonIndex = json.indexOf(':', keyIndex);
  if (colonIndex == -1) return "";

  int startQuote = json.indexOf('"', colonIndex + 1);
  if (startQuote == -1) {
    int endIndex = json.indexOf(',', colonIndex + 1);
    if (endIndex == -1) endIndex = json.indexOf('}', colonIndex + 1);
    if (endIndex == -1) endIndex = json.length();
    String raw = json.substring(colonIndex + 1, endIndex);
    raw.trim();
    return raw;
  }

  int endQuote = json.indexOf('"', startQuote + 1);
  if (endQuote == -1) return "";

  return json.substring(startQuote + 1, endQuote);
}

float extractJsonFloatValue(const String& json, const String& key) {
  String searchKey = "\"" + key + "\"";
  int keyIndex = json.indexOf(searchKey);
  if (keyIndex == -1) return 0.0;

  int colonIndex = json.indexOf(':', keyIndex);
  if (colonIndex == -1) return 0.0;

  int endIndex = json.indexOf(',', colonIndex + 1);
  if (endIndex == -1) endIndex = json.indexOf('}', colonIndex + 1);
  if (endIndex == -1) endIndex = json.length();

  String raw = json.substring(colonIndex + 1, endIndex);
  raw.trim();
  if (raw.length() == 0) return 0.0;
  return raw.toFloat();
}

bool isValidPlantResultJson(const String& json) {
  if (json.length() == 0) return false;

  const String requiredKeys[] = {
    "plantName",
    "assessment",
    "details",
    "confidence"
  };

  for (const String& key : requiredKeys) {
    if (json.indexOf("\"" + key + "\"") == -1) {
      return false;
    }
  }

  return true;
}

// ==============================
// HTTP route handlers
// ==============================
void handlePreflight() {
  setCorsHeaders();
  server.send(200, "text/plain", "OK");
}

void handleRoot() {
  setCorsHeaders();
  server.send(200, "text/plain", "Plant Disease Detector ESP32");
}

void handlePing() {
  setCorsHeaders();
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  setCorsHeaders();

  String payload = "{";
  payload += "\"status\":\"online\",";
  payload += "\"wifi\":\"connected\",";
  payload += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  payload += "}";

  server.send(200, "application/json", payload);
}

void handlePlantResult() {
  setCorsHeaders();

  String body = readRequestBody();

  Serial.println("\n--- Plant result received ---");
  Serial.println(body);

  if (body.length() == 0) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No JSON body received.\"}");
    return;
  }

  if (!isValidPlantResultJson(body)) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON payload. Required fields: plantName, assessment, details, confidence.\"}");
    return;
  }

  latestPlantName = extractJsonStringValue(body, "plantName");
  latestAssessment = extractJsonStringValue(body, "assessment");
  latestDetails = extractJsonStringValue(body, "details");
  latestConfidence = extractJsonFloatValue(body, "confidence");

  if (latestPlantName.length() == 0) latestPlantName = "Unknown";
  if (latestAssessment.length() == 0) latestAssessment = "Not provided";
  if (latestDetails.length() == 0) latestDetails = "No details provided.";

  Serial.printf("Plant name: %s\n", latestPlantName.c_str());
  Serial.printf("Assessment: %s\n", latestAssessment.c_str());
  Serial.printf("Confidence: %.2f\n", latestConfidence);
  Serial.printf("Details: %s\n", latestDetails.c_str());

  String responseJson = "{";
  responseJson += "\"status\":\"accepted\",";
  responseJson += "\"plantName\":\"" + latestPlantName + "\",";
  responseJson += "\"assessment\":\"" + latestAssessment + "\",";
  responseJson += "\"details\":\"" + latestDetails + "\",";
  responseJson += "\"confidence\":" + String(latestConfidence, 2);
  responseJson += "}";

  server.send(200, "application/json", responseJson);
}

void handleNotFound() {
  setCorsHeaders();
  server.send(404, "text/plain", "Not Found");
}

// ==============================
// Setup
// ==============================
void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi...");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected.");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connection failed. Check SSID and password.");
  }

  // Required CORS preflight support
  server.on("/status", HTTP_OPTIONS, handlePreflight);
  server.on("/ping", HTTP_OPTIONS, handlePreflight);
  server.on("/plant-result", HTTP_OPTIONS, handlePreflight);

  // Normal routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/ping", HTTP_GET, handlePing);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/plant-result", HTTP_POST, handlePlantResult);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}

// ==============================
// Loop
// ==============================
void loop() {
  server.handleClient();
}
