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

const int TOUCH_PIN = 15;

const int TOUCH_THRESHOLD = 800;

bool isTouching = false;


// =================================================
// NURSE BUTTONS
// =================================================

// Button 1 = Nurse starts attending
const int ATTEND_BUTTON = 4;

// Button 2 = Nurse finishes visit
const int DONE_BUTTON = 16;


// =================================================
// Button state variables
// =================================================

bool lastAttendState = HIGH;
bool lastDoneState = HIGH;


// =================================================
// Current status
// =================================================

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
}


// =================================================
// LOOP
// =================================================

void loop()
{

    // Keep WebSocket running
    webSocket.loop();


    // =================================================
    // 1. PATIENT TOUCH
    // =================================================

    int touchValue = touchRead(TOUCH_PIN);


    // Touch detected
    if(
        touchValue < TOUCH_THRESHOLD &&
        !isTouching
    )
    {

        isTouching = true;

        Serial.print("Touch detected: ");
        Serial.println(touchValue);


        // Send CALL to HTML
        sendStatus("CALL");
    }


    // Touch released
    if(touchValue >= TOUCH_THRESHOLD)
    {
        isTouching = false;
    }


    // =================================================
    // 2. NURSE ATTEND BUTTON
    // =================================================

    bool attendState = digitalRead(ATTEND_BUTTON);


    // Button pressed
    if(
        attendState == LOW &&
        lastAttendState == HIGH
    )
    {

        Serial.println("Nurse is attending patient.");

        sendStatus("ATTENDING");

        delay(50);   // Simple debounce
    }


    lastAttendState = attendState;


    // =================================================
    // 3. NURSE DONE BUTTON
    // =================================================

    bool doneState = digitalRead(DONE_BUTTON);


    // Button pressed
    if(
        doneState == LOW &&
        lastDoneState == HIGH
    )
    {

        Serial.println("Visit completed.");

        sendStatus("DONE");

        delay(50);   // Simple debounce
    }


    lastDoneState = doneState;
}