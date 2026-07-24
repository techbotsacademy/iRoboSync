#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const int LED_PIN = 2; // Built-in LED on most ESP32 boards

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<body>

<h2>ESP32 LED Control</h2>

<button onclick="toggleLED()">Toggle LED</button>

<script>
function toggleLED() {
    fetch('/toggle');
}
</script>

</body>
</html>
)rawliteral");
}

void handleToggle() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  server.send(200, "text/plain", "OK");
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin("SSID", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  server.begin();
}

void loop() {
  server.handleClient();
}