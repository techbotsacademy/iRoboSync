#include <LittleFS.h>

#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "AndroidAP";
const char* password = "12345678";

WebServer server(80);
String receivedImage = "";

void handleRoot()
{
    File file = LittleFS.open("/index.html", "r");

    if (!file)
    {
        server.send(404, "text/plain", "index.html not found");
        return;
    }

    server.streamFile(file, "text/html");
    file.close();
}

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed!");
    return;
  }

  Serial.println("LittleFS mounted!");

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
  server.on("/", handleRoot);

  // Automatic CORS enable karein (Doosre manual headers hata diye gaye hain)
  server.enableCORS(true);

  // OPTIONS handler for preflight requests
  server.on("/upload", HTTP_OPTIONS, []() {
    server.send(200, "text/plain", "OK");
  });

  // POST Handler
  server.on("/upload", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      receivedImage = server.arg("plain");
      Serial.printf("Image Successfully Received! Data Size: %d bytes\n", receivedImage.length());
      server.send(200, "text/plain", "SUCCESS");
    } else {
      server.send(400, "text/plain", "NO DATA");
    }
  });

  server.begin();
  Serial.println("Server Active and Ready!");
}

void loop() {
  server.handleClient();
}