#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi Credentials
const char* ssid     = "AndroidAP";
const char* password = "12345678";

// LED Pin (GPIO 2 is the built-in LED on most ESP32 boards)
const int LED_PIN = 2;

// Create WebServer object on port 80
WebServer server(80);

// Function to handle the home page
void handleRoot() {
    String html =
    "<!DOCTYPE html><html><head><title>ESP32 LED Control</title></head>"
    "<body>"
    "<h1>ESP32 LED Control</h1>"
    "<a href=\"/led/on\"><button style=\"padding:15px; font-size:20px; background-color:green; color:white;\">Turn ON</button></a> "
    "<a href=\"/led/off\"><button style=\"padding:15px; font-size:20px; background-color:red; color:white;\">Turn OFF</button></a>"
    "</body></html>";
 
  server.send(200, "text/html", html);
}

// Function to turn LED ON
void handleLedOn() {
  digitalWrite(LED_PIN, HIGH);
  server.sendHeader("Location", "/");
  server.send(303); // Redirect back to home page
}

// Function to turn LED OFF
void handleLedOff() {
  digitalWrite(LED_PIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303); // Redirect back to home page
}

void setup() {
  // Start Serial Communication
  Serial.begin(115200);

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print ESP32 IP Address
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define server routes
  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);

  // Start Server
  server.begin();
}

void loop() {
  // Handle client requests
  server.handleClient();
}