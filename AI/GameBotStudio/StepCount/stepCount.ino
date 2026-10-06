#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
//MPU6050 by Electronic Cats
#include <MPU6050.h>

#define SDA_PIN 19
#define SCL_PIN 18

// ==================================================
// WiFi Access Point
// ==================================================

const char* AP_SSID = "STEP_COUNTER_ESP32";
const char* AP_PASSWORD = "12345678";

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

// ==================================================
// Objects
// ==================================================

MPU6050 mpu;
WebServer server(80);

// ==================================================
// Step Counter Variables
// ==================================================

long stepCount = 0;

float lastMagnitude = 0.0;

unsigned long lastStepTime = 0;

const float STEP_THRESHOLD = 2500.0;
const unsigned long STEP_DELAY = 300;

// ==================================================
// CORS
// ==================================================

void addCorsHeaders()
{
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.sendHeader("Access-Control-Allow-Methods","GET");
  server.sendHeader("Access-Control-Allow-Headers","Content-Type");
}

// ==================================================
// API : Steps
// ==================================================

void handleSteps()
{
  addCorsHeaders();

  String json = "{";
  json += "\"steps\":";
  json += String(stepCount);
  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}

// ==================================================
// API : Reset
// ==================================================

void handleReset()
{
  addCorsHeaders();

  stepCount = 0;

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

// ==================================================
// Home Page
// ==================================================

void handleRoot()
{
  addCorsHeaders();

  server.send(
    200,
    "text/plain",
    "ESP32 Step Counter Running"
  );
}

// ==================================================
// WiFi
// ==================================================

void startAccessPoint()
{
  Serial.println();
  Serial.println("Starting WiFi...");

  WiFi.mode(WIFI_AP);

  WiFi.softAPConfig(
    local_IP,
    gateway,
    subnet
  );

  bool ok =
    WiFi.softAP(
      AP_SSID,
      AP_PASSWORD
    );

  if(!ok)
  {
    Serial.println("WiFi Failed");
    return;
  }

  delay(500);

  Serial.println();
  Serial.println("WiFi Started");

  Serial.print("SSID: ");
  Serial.println(AP_SSID);

  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

// ==================================================
// Server
// ==================================================

void startServer()
{
  server.on("/", handleRoot);

  server.on(
    "/api/steps",
    HTTP_GET,
    handleSteps
  );

  server.on(
    "/api/reset",
    HTTP_GET,
    handleReset
  );

  server.begin();

  Serial.println("Server Started");
}

// ==================================================
// Setup
// ==================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("========================");
  Serial.println("ESP32 STEP COUNTER");
  Serial.println("========================");

  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );

  Serial.println("Initializing MPU6050");

  mpu.initialize();

  Serial.println("MPU6050 Initialized");

  startAccessPoint();

  startServer();

  Serial.println();
  Serial.println("System Ready");
}

// ==================================================
// Loop
// ==================================================

void loop()
{
  server.handleClient();

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(
    &ax,
    &ay,
    &az,
    &gx,
    &gy,
    &gz
  );

  float magnitude =
    sqrt(
      (float)ax * ax +
      (float)ay * ay +
      (float)az * az
    );

  float change =
    fabs(
      magnitude -
      lastMagnitude
    );

  if(change > STEP_THRESHOLD)
  {
    if(
      millis() -
      lastStepTime >
      STEP_DELAY
    )
    {
      stepCount++;

      lastStepTime =
        millis();

      Serial.print("Steps: ");
      Serial.println(stepCount);
    }
  }

  lastMagnitude =
    magnitude;

  delay(20);
}

