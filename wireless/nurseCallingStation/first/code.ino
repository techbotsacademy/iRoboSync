#include <WiFi.h>
#include <WebServer.h>

//===========================
// WiFi
//===========================
const char* ssid = "ssid";
const char* password = "password";

//===========================
// Web Server
//===========================
WebServer server(80);

//===========================
// Touch Pins
//===========================
const int CALL_PIN      = 15;
const int ATTEND_PIN    = 4;
const int DONE_PIN      = 12;

int call_pin_threshold=0;
int attend_pin_threshold=0;
int done_pin_threshold=0;

const int TOUCH_THRESHOLD = 200;

//===========================
// Current Status
//===========================
String currentStatus = "WAITING";

// Prevent repeated triggering while finger is held
bool callPressed = false;
bool attendPressed = false;
bool donePressed = false;

//------------------------------------------------
void handleStatus()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET");
    server.sendHeader("Access-Control-Allow-Headers", "*");

    server.send(200, "text/plain", currentStatus);
}

//------------------------------------------------
void setup()
{
    Serial.begin(115200);

    WiFi.begin(ssid, password);

    Serial.println("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected");
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/status", HTTP_GET, handleStatus);

    server.begin();

    Serial.println("Web Server Started");

    call_pin_threshold=touchRead(CALL_PIN);
    attend_pin_threshold=touchRead(ATTEND_PIN);
    done_pin_threshold=touchRead(DONE_PIN);

    Serial.println(call_pin_threshold);
    Serial.println(attend_pin_threshold);
    Serial.println(done_pin_threshold);
}

//------------------------------------------------
void loop()
{
    server.handleClient();
    int call=touchRead(CALL_PIN);
    int attend=touchRead(ATTEND_PIN);
    int done=touchRead(DONE_PIN);
    // ---------------- CALL ----------------
    if ((call_pin_threshold-call) > TOUCH_THRESHOLD)
    {
        if (!callPressed)
        {
            callPressed = true;
            donePressed = false;

            currentStatus = "CALL";
            Serial.println("Patient Calling");
        }
    }

    // ---------------- ATTENDING ----------------
    if ((attend_pin_threshold-attend) > TOUCH_THRESHOLD)
    {
        if (!attendPressed & callPressed)
        {
            attendPressed = true;
            callPressed = false;
            currentStatus = "ATTENDING";
            Serial.println("Attending");
        }
    }

    // ---------------- DONE ----------------
    if ((done_pin_threshold-done) > TOUCH_THRESHOLD)
    {
        if (!donePressed and attendPressed)
        {
            attendPressed = false;
            donePressed = true;
            currentStatus = "DONE";
            Serial.println("Completed");
        }
    }
}