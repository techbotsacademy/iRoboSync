/*
  AI Activity Recognition
  ESP32 + MPU6050
  ------------------------------------------
  Activities:
  WALKING
  RUNNING
  SQUAT
  STANDING

  ESP32 creates its own Wi-Fi Access Point.
  Browser connects to:
  http://192.168.4.1

  Sensor API:
  GET http://192.168.4.1/api/status

  Sampling rate:
  50 Hz

  Sensor data:
  accX, accY, accZ
  gyroX, gyroY, gyroZ

  This ESP32 firmware only provides real sensor data.
  Machine learning is performed in the browser using TensorFlow.js.
*/

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ============================================================
// Wi-Fi Access Point Configuration
// ============================================================

const char* AP_SSID = "AI_ACTIVITY_ESP32";
const char* AP_PASSWORD = "AI12345678";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// ============================================================
// I2C Configuration
// ============================================================

#define SDA_PIN 19
#define SCL_PIN 18

// ============================================================
// Sampling Configuration
// ============================================================

const unsigned long SAMPLE_INTERVAL_MS = 20;  // 50 Hz

unsigned long lastSampleTime = 0;
unsigned long sampleCounter = 0;

// ============================================================
// Objects
// ============================================================

Adafruit_MPU6050 mpu;
WebServer server(80);

// ============================================================
// Latest Sensor Data
// ============================================================

float accX = 0.0f;
float accY = 0.0f;
float accZ = 0.0f;

float gyroX = 0.0f;
float gyroY = 0.0f;
float gyroZ = 0.0f;

unsigned long sensorTimestamp = 0;

// ============================================================
// MPU6050 Initialization
// ============================================================

bool initializeMPU6050() {

  Serial.println();
  Serial.println("Initializing MPU6050...");

  if (!mpu.begin()) {
    Serial.println("ERROR: MPU6050 not detected.");
    Serial.println("Please check the wiring:");
    Serial.println("VCC -> 3.3V");
    Serial.println("GND -> GND");
    Serial.println("SDA -> GPIO 19");
    Serial.println("SCL -> GPIO 18");

    return false;
  }

  Serial.println("MPU6050 initialized successfully.");

  // Accelerometer range
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // Gyroscope range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Digital low-pass filter
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);

  Serial.println("MPU6050 configuration:");
  Serial.println("Accelerometer: +/- 8G");
  Serial.println("Gyroscope: +/- 500 deg/s");
  Serial.println("Filter bandwidth: 21 Hz");
  Serial.println("Sampling rate: 50 Hz");

  return true;
}

// ============================================================
// Read MPU6050
// ============================================================

void readSensor() {

  sensors_event_t accelerometer;
  sensors_event_t gyroscope;
  sensors_event_t temperature;

  mpu.getEvent(
    &accelerometer,
    &gyroscope,
    &temperature
  );

  // Accelerometer values are in m/s^2
  accX = accelerometer.acceleration.x;
  accY = accelerometer.acceleration.y;
  accZ = accelerometer.acceleration.z;

  // Gyroscope values are in rad/s
  gyroX = gyroscope.gyro.x;
  gyroY = gyroscope.gyro.y;
  gyroZ = gyroscope.gyro.z;

  sensorTimestamp = millis();
  sampleCounter++;
}

// ============================================================
// CORS Headers
// ============================================================

void addCorsHeaders() {

  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  server.sendHeader(
    "Access-Control-Allow-Methods",
    "GET, OPTIONS"
  );

  server.sendHeader(
    "Access-Control-Allow-Headers",
    "Content-Type"
  );
}

// ============================================================
// Root Page
// ============================================================

void handleRoot() {

  addCorsHeaders();

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>AI Activity Recognition ESP32</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #111;
      color: white;
      text-align: center;
      padding: 30px;
    }

    h1 {
      color: #00ff88;
    }

    .sensor {
      display: inline-block;
      margin: 8px;
      padding: 15px;
      min-width: 100px;
      background: #222;
      border-radius: 10px;
    }

    .value {
      font-size: 20px;
      color: #00ccff;
    }
  </style>
</head>

<body>

  <h1>AI Activity Recognition</h1>

  <p>ESP32 + MPU6050 Sensor Server</p>

  <div class="sensor">
    <div>ACC X</div>
    <div class="value" id="ax">0</div>
  </div>

  <div class="sensor">
    <div>ACC Y</div>
    <div class="value" id="ay">0</div>
  </div>

  <div class="sensor">
    <div>ACC Z</div>
    <div class="value" id="az">0</div>
  </div>

  <br>

  <div class="sensor">
    <div>GYRO X</div>
    <div class="value" id="gx">0</div>
  </div>

  <div class="sensor">
    <div>GYRO Y</div>
    <div class="value" id="gy">0</div>
  </div>

  <div class="sensor">
    <div>GYRO Z</div>
    <div class="value" id="gz">0</div>
  </div>

  <script>

    async function updateSensor() {

      try {

        const response =
          await fetch('/api/status');

        const data =
          await response.json();

        document.getElementById('ax').textContent =
          data.ax.toFixed(3);

        document.getElementById('ay').textContent =
          data.ay.toFixed(3);

        document.getElementById('az').textContent =
          data.az.toFixed(3);

        document.getElementById('gx').textContent =
          data.gx.toFixed(3);

        document.getElementById('gy').textContent =
          data.gy.toFixed(3);

        document.getElementById('gz').textContent =
          data.gz.toFixed(3);

      } catch (error) {

        console.error(
          'Sensor request failed:',
          error
        );

      }

    }

    setInterval(updateSensor, 20);

    updateSensor();

  </script>

</body>
</html>
)rawliteral";

  server.send(
    200,
    "text/html; charset=utf-8",
    html
  );
}

// ============================================================
// Sensor API
// ============================================================

void handleSensorStatus() {

  addCorsHeaders();

  // Read fresh sensor data before sending the response.
  readSensor();

  String json = "{";

  json += "\"ax\":";
  json += String(accX, 6);

  json += ",\"ay\":";
  json += String(accY, 6);

  json += ",\"az\":";
  json += String(accZ, 6);

  json += ",\"gx\":";
  json += String(gyroX, 6);

  json += ",\"gy\":";
  json += String(gyroY, 6);

  json += ",\"gz\":";
  json += String(gyroZ, 6);

  json += ",\"timestamp\":";
  json += String(sensorTimestamp);

  json += ",\"sample\":";
  json += String(sampleCounter);

  json += ",\"sampleRate\":50";

  json += "}";

  server.send(
    200,
    "application/json",
    json
  );
}

// ============================================================
// OPTIONS Request
// ============================================================

void handleOptions() {

  addCorsHeaders();

  server.send(
    204,
    "text/plain",
    ""
  );
}

// ============================================================
// Not Found Handler
// ============================================================

void handleNotFound() {

  addCorsHeaders();

  String message;

  message += "AI Activity Recognition ESP32\n\n";
  message += "Endpoint not found.\n\n";
  message += "Available endpoints:\n";
  message += "/\n";
  message += "/api/status\n";

  server.send(
    404,
    "text/plain",
    message
  );
}

// ============================================================
// Start Wi-Fi Access Point
// ============================================================

void startAccessPoint() {

  Serial.println();
  Serial.println("Starting Wi-Fi Access Point...");

  WiFi.mode(WIFI_AP);

  if (!WiFi.softAPConfig(
        local_IP,
        gateway,
        subnet
      )) {

    Serial.println(
      "ERROR: Failed to configure Access Point."
    );
  }

  bool apStarted = WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );

  if (!apStarted) {

    Serial.println(
      "ERROR: Failed to start Wi-Fi Access Point."
    );

    return;
  }

  delay(500);

  Serial.println();
  Serial.println("Wi-Fi Access Point started.");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);

  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("Connected stations: ");
  Serial.println(WiFi.softAPgetStationNum());
}

// ============================================================
// Start HTTP Server
// ============================================================

void startWebServer() {

  // Main page
  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );

  // Sensor API
  server.on(
    "/api/status",
    HTTP_GET,
    handleSensorStatus
  );

  // CORS preflight
  server.on(
    "/api/status",
    HTTP_OPTIONS,
    handleOptions
  );

  // Not found
  server.onNotFound(
    handleNotFound
  );

  server.begin();

  Serial.println();
  Serial.println("HTTP server started.");
  Serial.println(
    "Open http://192.168.4.1 in a browser."
  );
}

// ============================================================
// Print Sensor Data to Serial Monitor
// ============================================================

void printSensorData() {

  static unsigned long lastSerialPrint = 0;

  // Print approximately every 500 ms.
  if (millis() - lastSerialPrint < 500) {
    return;
  }

  lastSerialPrint = millis();

  Serial.print("ACC: ");

  Serial.print(accX, 2);
  Serial.print(", ");

  Serial.print(accY, 2);
  Serial.print(", ");

  Serial.print(accZ, 2);

  Serial.print(" | GYRO: ");

  Serial.print(gyroX, 2);
  Serial.print(", ");

  Serial.print(gyroY, 2);
  Serial.print(", ");

  Serial.println(gyroZ, 2);
}

// ============================================================
// Setup
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println();
  Serial.println("========================================");
  Serial.println("      AI ACTIVITY RECOGNITION");
  Serial.println("      ESP32 + MPU6050");
  Serial.println("========================================");

  Serial.println();
  Serial.println("Starting system...");

  // ----------------------------------------------------------
  // I2C
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("Initializing I2C...");

  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );

  Serial.print("SDA Pin: GPIO ");
  Serial.println(SDA_PIN);

  Serial.print("SCL Pin: GPIO ");
  Serial.println(SCL_PIN);

  // ----------------------------------------------------------
  // MPU6050
  // ----------------------------------------------------------

  if (!initializeMPU6050()) {

    Serial.println();
    Serial.println(
      "SYSTEM ERROR: MPU6050 initialization failed."
    );

    Serial.println(
      "The system will continue, but sensor data will not be valid."
    );

  } else {

    Serial.println(
      "MPU6050 is ready."
    );
  }

  // ----------------------------------------------------------
  // Wi-Fi Access Point
  // ----------------------------------------------------------

  startAccessPoint();

  // ----------------------------------------------------------
  // HTTP Server
  // ----------------------------------------------------------

  startWebServer();

  Serial.println();
  Serial.println("Sensor streaming started.");
  Serial.println("Sampling rate: 50 Hz");
  Serial.println();
  Serial.println("System ready.");
  Serial.println("========================================");
}

// ============================================================
// Main Loop
// ============================================================

void loop() {

  // Handle browser HTTP requests.
  server.handleClient();

  // Keep a regular 50 Hz sensor sampling loop.
  unsigned long currentTime = millis();

  if (
    currentTime - lastSampleTime >=
    SAMPLE_INTERVAL_MS
  ) {

    lastSampleTime = currentTime;

    readSensor();

    printSensorData();
  }
}
