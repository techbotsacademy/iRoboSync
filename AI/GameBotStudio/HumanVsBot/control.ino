#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>

const char* ssid = "user_name";
const char* password = "password";

WebServer server(80);

Servo elbowServo;

const int elbowPin = 14;

int currentAngle = 90;
int targetAngle = 90;

void handleMove() {

  if (server.hasArg("y")) {

    int y = server.arg("y").toInt();

    // Game canvas is 380 pixels high
    targetAngle = map(y, 0, 380, 10, 125);
    targetAngle = constrain(targetAngle, 10, 125);

    Serial.print("Target Y: ");
    Serial.print(y);
    Serial.print("  Servo Target: ");
    Serial.println(targetAngle);

    server.send(200, "text/plain", "OK");
  }
  else {
    server.send(400, "text/plain", "No y");
  }
}

void setup() {

  Serial.begin(115200);

  elbowServo.attach(elbowPin);
  elbowServo.write(currentAngle);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.enableCORS();
  server.on("/move", HTTP_GET, handleMove);
  server.begin();
}

void loop() {

  server.handleClient();

  // Smooth movement
  if (currentAngle < targetAngle) {
    currentAngle++;
    elbowServo.write(currentAngle);
  }
  else if (currentAngle > targetAngle) {
    currentAngle--;
    elbowServo.write(currentAngle);
  }
}