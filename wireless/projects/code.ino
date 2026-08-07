#include <WiFi.h>
#include <WebServer.h>

// ===========================
// WiFi Credentials
// ===========================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ===========================
// Web Server
// ===========================
WebServer server(80);

// ===========================
// Touch Pin
// ===========================
const int TOUCH_PIN = 15;
const int TOUCH_THRESHOLD = 800;

// ===========================
// Touch Variables
// ===========================
int touchCount = 0;
unsigned long lastTouchTime = 0;
const int TAP_TIMEOUT = 400;
bool isTouching = false;

// Current nurse status
String currentStatus = "WAITING";

// ------------------------------------------------
// Return current status to HTML
// ------------------------------------------------
void handleStatus()
{
    server.send(200, "text/plain", currentStatus);
}

void setup()
{
    Serial.begin(115200);

    WiFi.begin(ssid, password);

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/status", handleStatus);

    server.begin();

    Serial.println("Web Server Started");
}

void loop()
{
    server.handleClient();

    int touchValue = touchRead(TOUCH_PIN);

    // Detect touch
    if (touchValue < TOUCH_THRESHOLD && !isTouching)
    {
        touchCount++;
        lastTouchTime = millis();
        isTouching = true;
        delay(40);
    }
    else if (touchValue >= TOUCH_THRESHOLD)
    {
        isTouching = false;
    }

    // Detect number of taps
    if (touchCount > 0 &&
        millis() - lastTouchTime > TAP_TIMEOUT)
    {
        Serial.print("Total Taps: ");
        Serial.println(touchCount);

        if (touchCount == 1)
        {
            currentStatus = "CALL";
            Serial.println("Patient Called Nurse");
        }
        else if (touchCount == 2)
        {
            currentStatus = "ATTENDING";
            Serial.println("Nurse Attending");
        }
        else if (touchCount >= 3)
        {
            currentStatus = "DONE";
            Serial.println("Visit Completed");
        }

        touchCount = 0;
    }
}