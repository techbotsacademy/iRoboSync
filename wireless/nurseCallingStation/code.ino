#include <WiFi.h>
#include <WebSocketsServer.h>

// =================================================
// WiFi
// =================================================

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";


// =================================================
// WebSocket Server
// Port 81
// =================================================

WebSocketsServer webSocket = WebSocketsServer(81);


// =================================================
// TOUCH SENSOR
// =================================================

const int CALL_PIN      = 15;
const int ATTEND_PIN    = 4;
const int DONE_PIN      = 12;

int call_pin_threshold=0;
int attend_pin_threshold=0;
int done_pin_threshold=0;

const int TOUCH_THRESHOLD = 200;
// Prevent repeated triggering while finger is held
bool callPressed = false;
bool attendPressed = false;
bool donePressed = false;
String currentStatus = "WAITING";


// =================================================
// Send status to all connected HTML dashboards
// =================================================

void sendStatus(String status)
{
    currentStatus = status;

    Serial.print("Sending: ");
    Serial.println(status);

    webSocket.broadcastTXT(status);
}


// =================================================
// WebSocket Events
// =================================================

void webSocketEvent(
    uint8_t num,
    WStype_t type,
    uint8_t * payload,
    size_t length
)
{
    switch(type)
    {

        // -----------------------------------------
        // Browser connected
        // -----------------------------------------

        case WStype_CONNECTED:

            Serial.print("HTML connected. Client #");
            Serial.println(num);

            // Send current status immediately
            webSocket.sendTXT(num, currentStatus);

            break;


        // -----------------------------------------
        // Browser disconnected
        // -----------------------------------------

        case WStype_DISCONNECTED:

            Serial.print("HTML disconnected. Client #");
            Serial.println(num);

            break;


        // -----------------------------------------
        // Message received from browser
        // -----------------------------------------

        case WStype_TEXT:

            Serial.print("Message from HTML: ");
            Serial.println((char*)payload);

            break;


        default:
            break;
    }
}


// =================================================
// SETUP
// =================================================

void setup()
{
    Serial.begin(115200);


    // -----------------------------------------
    // Touch
    // -----------------------------------------

    Serial.println();
    Serial.println("Starting Smart Nurse Station...");


    // -----------------------------------------
    // Nurse buttons
    // -----------------------------------------

    pinMode(ATTEND_BUTTON, INPUT_PULLUP);

    pinMode(DONE_BUTTON, INPUT_PULLUP);


    // -----------------------------------------
    // Connect WiFi
    // -----------------------------------------

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);

        Serial.print(".");
    }


    Serial.println();

    Serial.println("WiFi Connected!");

    Serial.print("ESP32 IP Address: ");

    Serial.println(WiFi.localIP());


    // -----------------------------------------
    // Start WebSocket server
    // -----------------------------------------

    webSocket.begin();

    webSocket.onEvent(webSocketEvent);

    Serial.println("WebSocket server started.");

    Serial.println("Waiting for HTML dashboard...");

    call_pin_threshold=touchRead(CALL_PIN);
    attend_pin_threshold=touchRead(ATTEND_PIN);
    done_pin_threshold=touchRead(DONE_PIN);
}


// =================================================
// LOOP
// =================================================

void loop()
{

    // Keep WebSocket running
    webSocket.loop();

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
            sendStatus("CALL");
        }
    }

    // ---------------- ATTENDING ----------------
    if ((attend_pin_threshold-attend) > TOUCH_THRESHOLD)
    {
        if (!attendPressed & callPressed)
        {
            attendPressed = true;
            callPressed = false;
            sendStatus("ATTENDING");
        }
    }

    // ---------------- DONE ----------------
    if ((done_pin_threshold-done) > TOUCH_THRESHOLD)
    {
        if (!donePressed and attendPressed)
        {
            attendPressed = false;
            donePressed = true;
            sendStatus("DONE");
        }
    }
}