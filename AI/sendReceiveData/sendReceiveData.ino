#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid     = "AndroidAP";
const char* password = "12345678";

WebServer server(80);
String storedImage = "";

// Function to serve index.html from LittleFS
void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "index.html not found in LittleFS!");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS File System
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed!");
    return;
  }
  Serial.println("LittleFS Mounted Successfully!");

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Enable CORS
  server.enableCORS(true);

  // Serve LittleFS index.html on Root URL
  server.on("/", HTTP_GET, handleRoot);

  // OPTIONS Preflight Handlers for CORS
  server.on("/upload", HTTP_OPTIONS, []() {
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/get-photo", HTTP_OPTIONS, []() {
    server.send(200, "text/plain", "OK");
  });

  // POST: Receive image from Laptop A
  server.on("/upload", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      storedImage = server.arg("plain");
      Serial.printf("Photo Stored from Laptop A! Size: %d bytes\n", storedImage.length());
      server.send(200, "text/plain", "SAVED");
    } else {
      server.send(400, "text/plain", "NO DATA");
    }
  });

  // GET: Send image to Laptop B
  server.on("/get-photo", HTTP_GET, []() {
    if (storedImage.length() > 0) {
      server.send(200, "text/plain", storedImage);
    } else {
      server.send(200, "text/plain", "EMPTY");
    }
  });

  server.begin();
  Serial.println("Server Ready!");
}

void loop() {
  server.handleClient();
}