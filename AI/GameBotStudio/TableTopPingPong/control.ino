#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>

// WiFi Details
const char* ssid = "GoSpaze_WFD";
const char* password = "9845454344";

WebServer server(80);

// Servo Objects
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo clawServo;

// Pins
const int basePin = 13;
const int shoulderPin = 12;
const int elbowPin = 14;
const int clawPin = 27;

void handleMove() {
  if (server.hasArg("y")) {
    String yValue = server.arg("y");
    int y = yValue.toInt();

    // Map the Y coordinate (0 to 400 from Canvas) to Servo degrees (0 to 180)
    // 400 is the height of your canvas
    int angle = map(y, 0, 400, 0, 180); 
    
    // Constraints to prevent damage
    angle = constrain(angle, 10, 170);

    // Sirf Base servo move kar rahe hain kyunki wahi ball ko follow karega
    baseServo.write(angle);
    
    // Baki servos ko ek fixed position pe rakhte hain (Table Tennis posture)
    shoulderServo.write(90); 
    elbowServo.write(90);
    clawServo.write(100); // Claw closed to hold paddle

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing Y");
  }
}

void setup() {
  Serial.begin(115200);

  // Attach Servos
  baseServo.attach(basePin);
  shoulderServo.attach(shoulderPin);
  elbowServo.attach(elbowPin);
  clawServo.attach(clawPin);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Is IP ko copy karke browser prompt mein dalein

  // API Route
  server.on("/move", HTTP_GET, handleMove);
  
  // Enable CORS (taki browser request block na kare)
  server.enableCORS();
  
  server.begin();
}

void loop() {
  server.handleClient();
}

