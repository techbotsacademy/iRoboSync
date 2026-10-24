#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include <ESP_Mail_Client.h>

// ==============================
// Wi-Fi Configuration
// ==============================
const char* WIFI_SSID = "OnePlus 7 Pro";
const char* WIFI_PASSWORD = "12345678";

// ==============================
// Gmail SMTP Configuration
// ==============================
const char* SMTP_HOST = "smtp.gmail.com";
const int SMTP_PORT = 465;

const char* SMTP_SENDER_EMAIL = "academytechbots@gmail.com";
const char* SMTP_SENDER_NAME = "Plant Disease Detector";
const char* SMTP_APP_PASSWORD = "ukmmjlfkogrtgujs";
const char* SMTP_RECIPIENT_EMAIL = "ishashek2001@example.com";

// ==============================
// Web Server
// ==============================
WebServer server(80);

String latestPlantName = "Not available";
String latestAssessment = "Not available";
String latestDetails = "No result received yet.";
float latestConfidence = 0.0;

// ==============================
// CORS
// ==============================
void setCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, Api-Key");
}

// ==============================
// JSON Helpers
// ==============================
String readRequestBody() {
  return server.hasArg("plain") ? server.arg("plain") : "";
}

String escapeJsonString(String value) {
  String output;

  for (size_t i = 0; i < value.length(); i++) {
    char c = value[i];

    if (c == '\\' || c == '"' || c == '\n' ||
        c == '\r' || c == '\t') {
      output += '\\';
    }

    if (c == '\n') {
      output += 'n';
    } else if (c == '\r') {
      output += 'r';
    } else if (c == '\t') {
      output += 't';
    } else {
      output += c;
    }
  }

  return output;
}

String extractJsonStringValue(const String& json, const String& key) {
  String searchKey = "\"" + key + "\"";
  int keyIndex = json.indexOf(searchKey);

  if (keyIndex < 0) return "";

  int colonIndex = json.indexOf(':', keyIndex);
  if (colonIndex < 0) return "";

  int startQuote = json.indexOf('"', colonIndex + 1);
  if (startQuote < 0) return "";

  // Find the closing quote, ignoring escaped quotes.
  int endQuote = startQuote + 1;

  while (endQuote < json.length()) {
    if (json[endQuote] == '"' && json[endQuote - 1] != '\\') {
      break;
    }
    endQuote++;
  }

  if (endQuote >= json.length()) return "";

  String value = json.substring(startQuote + 1, endQuote);
  value.replace("\\n", "\n");
  value.replace("\\r", "\r");
  value.replace("\\t", "\t");
  value.replace("\\\"", "\"");
  value.replace("\\\\", "\\");

  return value;
}

float extractJsonFloatValue(const String& json, const String& key) {
  String searchKey = "\"" + key + "\"";
  int keyIndex = json.indexOf(searchKey);

  if (keyIndex < 0) return 0.0;

  int colonIndex = json.indexOf(':', keyIndex);
  if (colonIndex < 0) return 0.0;

  int endIndex = json.indexOf(',', colonIndex + 1);

  if (endIndex < 0) {
    endIndex = json.indexOf('}', colonIndex + 1);
  }

  if (endIndex < 0) endIndex = json.length();

  String raw = json.substring(colonIndex + 1, endIndex);
  raw.trim();

  return raw.toFloat();
}

bool hasRequiredKeys(const String& json) {
  return json.indexOf("\"plantName\"") >= 0 &&
         json.indexOf("\"assessment\"") >= 0 &&
         json.indexOf("\"details\"") >= 0 &&
         json.indexOf("\"confidence\"") >= 0;
}

String normalizeAssessment(String assessment) {
  assessment.trim();
  assessment.toLowerCase();

  if (assessment.indexOf("healthy") >= 0) {
    return "Healthy";
  }

  if (assessment.indexOf("disease") >= 0 ||
      assessment.indexOf("pest") >= 0 ||
      assessment.indexOf("unhealthy") >= 0 ||
      assessment.indexOf("possible") >= 0) {
    return "Possible Disease Detected";
  }

  return assessment.length() ? assessment : "Unknown";
}

String getCurrentDateTimeString() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo, 3000)) {
    return "Time unavailable";
  }

  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  return String(buffer) + " UTC";
}

// ==============================
// Send Email
// ==============================
bool sendPlantReportEmail(
  const String& plantName,
  const String& assessment,
  const String& details,
  float confidence
) {
  Serial.println("Preparing plant report email...");

  SMTPSession smtp;
  smtp.debug(1);

  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = SMTP_SENDER_EMAIL;
  config.login.password = SMTP_APP_PASSWORD;

  SMTP_Message message;
  message.sender.name = SMTP_SENDER_NAME;
  message.sender.email = SMTP_SENDER_EMAIL;
  message.subject = "Plant Disease Detection Report";

  message.addRecipient(
    "Plant Report Recipient",
    SMTP_RECIPIENT_EMAIL
  );

  float confidencePercent = confidence;

  if (confidence > 0.0 && confidence <= 1.0) {
    confidencePercent = confidence * 100.0;
  }

  String confidenceText = confidence > 0.0
      ? String(confidencePercent, 2) + "%"
      : "Not available";

  String body;
  body += "Plant Disease Detection Report\n";
  body += "================================\n\n";
  body += "Plant name: " + plantName + "\n";
  body += "Assessment: " + normalizeAssessment(assessment) + "\n";
  body += "Details: " + details + "\n";
  body += "Confidence: " + confidenceText + "\n";
  body += "Received at: " + getCurrentDateTimeString() + "\n";

  message.text.content = body.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding =
      Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&config)) {
    Serial.println("SMTP connection failed.");
    Serial.printf("SMTP status code: %d\n", smtp.statusCode());
Serial.printf("SMTP error code: %d\n", smtp.errorCode());
    Serial.println(smtp.errorReason());
    return false;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Email send failed.");
    Serial.println(smtp.errorReason());
    smtp.closeSession();
    return false;
  }

  smtp.closeSession();
  Serial.println("Email sent successfully.");
  return true;
}

// ==============================
// HTTP Handlers
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

  if (body.length() == 0 || !hasRequiredKeys(body)) {
    server.send(
      400,
      "application/json",
      "{\"received\":false,\"emailSent\":false,"
      "\"message\":\"Missing required JSON fields.\"}"
    );
    return;
  }

  latestPlantName = extractJsonStringValue(body, "plantName");
  latestAssessment = extractJsonStringValue(body, "assessment");
  latestDetails = extractJsonStringValue(body, "details");
  latestConfidence = extractJsonFloatValue(body, "confidence");

  if (latestPlantName.length() == 0) {
    latestPlantName = "Unknown";
  }

  if (latestAssessment.length() == 0) {
    latestAssessment = "Unknown";
  }

  if (latestDetails.length() == 0) {
    latestDetails = "No details provided.";
  }

  Serial.println("Plant: " + latestPlantName);
  Serial.println("Assessment: " + latestAssessment);
  Serial.println("Details: " + latestDetails);
  Serial.printf("Confidence: %.2f\n", latestConfidence);

  bool emailSent = false;

  if (WiFi.status() == WL_CONNECTED) {
    emailSent = sendPlantReportEmail(
      latestPlantName,
      latestAssessment,
      latestDetails,
      latestConfidence
    );
  } else {
    Serial.println("Wi-Fi disconnected; email not sent.");
  }

  String response = "{";
  response += "\"received\":true,";
  response += "\"emailSent\":";
  response += emailSent ? "true" : "false";
  response += ",\"message\":\"";
  response += emailSent
      ? "Plant result received and email sent."
      : "Plant result received, but email sending failed.";
  response += "\"}";

  server.send(200, "application/json", response);
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

  Serial.println("Connecting to Wi-Fi...");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected.");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Try synchronizing the clock with internet time servers.
    configTime(0, 0, "pool.ntp.org", "time.google.com");

    Serial.println("Waiting for internet time...");

    struct tm timeinfo;
    bool timeReady = false;

    for (int i = 0; i < 30; i++) {
      if (getLocalTime(&timeinfo, 1000)) {
        timeReady = true;
        break;
      }
      delay(500);
      Serial.print(".");
    }

    if (timeReady) {
      Serial.println("\nClock synchronized successfully.");
      Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S UTC");
    } else {
      Serial.println("\nClock sync failed.");
      Serial.println("Check internet access and NTP restrictions.");
    }

    // Test whether Gmail SMTP port 465 is reachable.
    Serial.println("Testing Gmail SMTP connection...");

    WiFiClient smtpTest;
    smtpTest.setTimeout(10);

    if (smtpTest.connect(SMTP_HOST, SMTP_PORT)) {
      Serial.println("Gmail SMTP port 465 is reachable.");
      smtpTest.stop();
    } else {
      Serial.println("Cannot reach Gmail SMTP port 465.");
      Serial.println("Network, DNS or firewall may be blocking it.");
    }

  } else {
    Serial.println("\nWi-Fi connection failed.");
    Serial.println("Check Wi-Fi credentials and signal.");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/ping", HTTP_GET, handlePing);
  server.on("/status", HTTP_GET, handleStatus);

  server.on("/ping", HTTP_OPTIONS, handlePreflight);
  server.on("/status", HTTP_OPTIONS, handlePreflight);
  server.on("/plant-result", HTTP_OPTIONS, handlePreflight);
  server.on("/plant-result", HTTP_POST, handlePlantResult);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}


