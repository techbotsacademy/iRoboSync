/*
 * =====================================================================================
 *  PROJECT: AI Elder Emergency Alert System
 *  FRAMEWORK: Arduino ESP32 Core
 *
 *  FEATURES
 *  -------------------------------------------------------------------------------------
 *  ✔ LittleFS Hosted HTML Dashboard
 *  ✔ Built-in HTTP Server
 *  ✔ Telegram Emergency Notification
 *  ✔ REST API
 *  ✔ JSON Status API
 *  ✔ WiFi Auto Reconnect
 *  ✔ Non-blocking Alarm
 * =====================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// =====================================================================================
// USER CONFIGURATION
// =====================================================================================

// WiFi Credentials
String WIFI_SSID;
String WIFI_PASSWORD;

String BOT_TOKEN;
String CHAT_ID;

// Hardware Pins
const uint8_t LED_PIN = 2;
const uint8_t BUZZER_PIN = 15;

// Alarm Duration
const unsigned long ALARM_DURATION_MS = 5000;

// =====================================================================================
// GLOBAL OBJECTS
// =====================================================================================

WebServer server(80);
WiFiClientSecure secureClient;
UniversalTelegramBot* bot = nullptr;

// =====================================================================================
// GLOBAL VARIABLES
// =====================================================================================

bool alarmActive = false;
unsigned long alarmStartTime = 0;

unsigned long totalEmergencyCount = 0;

String lastDetectedPhrase = "None";
String lastAlertTimestamp = "Never";

// =====================================================================================
// HELPER FUNCTIONS
// =====================================================================================

bool loadConfig()
{
    File file = LittleFS.open("/config.env", "r");

    if (!file)
    {
        Serial.println("config.env not found");
        return false;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');

        line.trim();

        if (line.length() == 0)
            continue;

        if (line.startsWith("#"))
            continue;

        int pos = line.indexOf('=');

        if (pos == -1)
            continue;

        String key = line.substring(0, pos);
        String value = line.substring(pos + 1);

        key.trim();
        value.trim();

        if (key == "SSID")
            WIFI_SSID = value;

        else if (key == "PASSWORD")
            WIFI_PASSWORD = value;

        else if (key == "BOT_TOKEN")
            BOT_TOKEN = value;

        else if (key == "CHAT_ID")
            CHAT_ID = value;
    }

    file.close();

    Serial.println("Configuration Loaded");

    return true;
}

String getSystemUptimeFormatted()
{
    unsigned long totalSeconds = millis() / 1000;

    unsigned int hours = totalSeconds / 3600;
    unsigned int minutes = (totalSeconds % 3600) / 60;
    unsigned int seconds = totalSeconds % 60;

    char buffer[20];

    sprintf(buffer,
            "%02u:%02u:%02u",
            hours,
            minutes,
            seconds);

    return String(buffer);
}

// =====================================================================================

void connectWiFi()
{
    Serial.println();
    Serial.println("======================================");
    Serial.println("Connecting to WiFi...");
    Serial.println("======================================");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected.");

    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());

    Serial.println();
}

// =====================================================================================

void checkAndMaintainWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    Serial.println();
    Serial.println("WiFi Lost.");
    Serial.println("Attempting reconnect...");

    WiFi.disconnect(true);

    delay(500);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 10000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.println("WiFi Reconnected.");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println();
        Serial.println("Reconnect Failed.");
    }
}

// =====================================================================================

bool mountFileSystem()
{
    Serial.println("Mounting LittleFS...");

    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS Mount Failed.");
        return false;
    }

    Serial.println("LittleFS Mounted Successfully.");
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    Serial.println("Files in LittleFS:");

    while (file)
    {
        Serial.printf("'%s'\n", file.name());
        file = root.openNextFile();
    }
    return true;
}

// =====================================================================================

bool fileExists(const char *path)
{
    File file = LittleFS.open(path, "r");

    if (!file)
        return false;

    file.close();

    return true;
}

// =====================================================================================

void serveFile(const char *path,
               const char *contentType)
{
    File file = LittleFS.open(path, "r");

    if (!file)
    {
        server.send(404,
                    "text/plain",
                    "Requested file not found.");
        return;
    }

    server.streamFile(file, contentType);

    file.close();
}

// =====================================================================================

String getLocalIPAddress()
{
    if (WiFi.status() != WL_CONNECTED)
        return "Disconnected";

    return WiFi.localIP().toString();
}

// =====================================================================================

String getSystemState()
{
    if (alarmActive)
        return "EMERGENCY ACTIVE";

    return "Idle / Listening";
}

// =====================================================================================
// TELEGRAM NOTIFICATION
// =====================================================================================

bool sendTelegramAlert(const String &phrase)
{
    secureClient.setInsecure();
    if (bot == nullptr)
    {
        bot = new UniversalTelegramBot(
            BOT_TOKEN,
            secureClient
        );
    }

    String message;

    message += "🚨 *EMERGENCY ALERT* 🚨\n\n";
    message += "An elderly person requires immediate assistance.\n\n";
    message += "🗣 *Trigger Phrase:* ";
    message += phrase;
    message += "\n";

    message += "⏱ *Uptime:* ";
    message += getSystemUptimeFormatted();
    message += "\n";

    message += "🌐 *IP:* ";
    message += getLocalIPAddress();
    message += "\n";

    message += "📟 *Device:* ESP32-AI-ELDER-NODE-01";

    Serial.println("--------------------------------------");
    Serial.println("Sending Telegram Alert...");
    Serial.println("--------------------------------------");

    const uint8_t MAX_RETRIES = 3;

    for (uint8_t i = 0; i < MAX_RETRIES; i++)
    {
        if (bot->sendMessage(CHAT_ID, message, "Markdown"))
        {
            Serial.println("Telegram Alert Sent.");
            return true;
        }

        Serial.print("Retry ");
        Serial.println(i + 1);

        delay(1000);
    }

    Serial.println("Telegram Failed.");

    return false;
}

// =====================================================================================
// HTTP ROUTES
// =====================================================================================

void handleRootPage()
{
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    if (!fileExists("/index.html"))
    {
        server.send(
            404,
            "text/plain",
            "index.html not found in LittleFS");

        return;
    }

    serveFile("/index.html", "text/html");
}

// =====================================================================================

void handleGetStatus()
{
    StaticJsonDocument<256> doc;

    doc["system"] = getSystemState();
    doc["ip"] = getLocalIPAddress();
    doc["last_phrase"] = lastDetectedPhrase;
    doc["last_alert"] = lastAlertTimestamp;
    doc["total_alerts"] = totalEmergencyCount;
    doc["uptime"] = getSystemUptimeFormatted();

    String response;

    serializeJson(doc, response);

    server.send(
        200,
        "application/json",
        response);
}

// =====================================================================================

void handleEmergencyTrigger()
{
    String detectedPhrase = "Unknown Trigger";

    if (server.hasArg("plain"))
    {
        StaticJsonDocument<256> doc;

        DeserializationError err =
            deserializeJson(doc,
                            server.arg("plain"));

        if (!err)
        {
            if (doc.containsKey("phrase"))
            {
                detectedPhrase =
                    doc["phrase"].as<String>();
            }
        }
    }
    else if (server.hasArg("phrase"))
    {
        detectedPhrase =
            server.arg("phrase");
    }

    Serial.println();
    Serial.println("====================================");
    Serial.println("EMERGENCY TRIGGER RECEIVED");
    Serial.println("====================================");

    Serial.print("Phrase : ");
    Serial.println(detectedPhrase);

    totalEmergencyCount++;

    lastDetectedPhrase = detectedPhrase;
    lastAlertTimestamp = getSystemUptimeFormatted();

    alarmActive = true;
    alarmStartTime = millis();

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    bool telegramSent =
        sendTelegramAlert(detectedPhrase);

    StaticJsonDocument<256> response;

    response["status"] = "success";
    response["telegram"] =
        telegramSent ? "sent" : "failed";
    response["alerts"] = totalEmergencyCount;

    String json;

    serializeJson(response, json);

    server.sendHeader(
        "Access-Control-Allow-Origin",
        "*");

    server.send(
        200,
        "application/json",
        json);
}

// =====================================================================================

void handleNotFound()
{
    StaticJsonDocument<128> doc;

    doc["status"] = 404;
    doc["error"] = "Endpoint Not Found";
    doc["path"] = server.uri();

    String json;

    serializeJson(doc, json);

    server.send(
        404,
        "application/json",
        json);
}

// =====================================================================================
// REGISTER ROUTES
// =====================================================================================

void configureServerRoutes()
{
    server.on(
        "/",
        HTTP_GET,
        handleRootPage);

    server.on(
        "/status",
        HTTP_GET,
        handleGetStatus);

    server.on(
        "/emergency",
        HTTP_GET,
        handleEmergencyTrigger);

    server.on(
        "/emergency",
        HTTP_POST,
        handleEmergencyTrigger);

    server.onNotFound(
        handleNotFound);
}

// =====================================================================================
// SETUP
// =====================================================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=======================================================");
    Serial.println("      AI Elder Emergency Alert System Starting");
    Serial.println("=======================================================");

    // ----------------------------------------------------
    // Configure Hardware
    // ----------------------------------------------------

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    // ----------------------------------------------------
    // Mount LittleFS
    // ----------------------------------------------------

    if (!mountFileSystem())
    {
        Serial.println("System halted because LittleFS could not be mounted.");

        while (true)
        {
            delay(1000);
        }
    }

    if (!loadConfig())
    {
        Serial.println("Cannot continue without config.");

        while (true);
    }

    // Check whether the HTML page exists

    if (fileExists("/index.html"))
    {
        Serial.println("index.html found.");
    }
    else
    {
        Serial.println("WARNING: index.html NOT found!");
    }

    // ----------------------------------------------------
    // Connect WiFi
    // ----------------------------------------------------

    connectWiFi();

    // ----------------------------------------------------
    // Telegram SSL Client
    // ----------------------------------------------------

    secureClient.setInsecure();

    // ----------------------------------------------------
    // Configure Web Server
    // ----------------------------------------------------

    configureServerRoutes();

    server.begin();

    Serial.println();
    Serial.println("HTTP Server Started.");
    Serial.println();

    Serial.print("Open Browser : http://");
    Serial.println(WiFi.localIP());

    Serial.println();
    Serial.println("REST API");
    Serial.println("-----------------------------------");
    Serial.println("/status");
    Serial.println("/emergency");
    Serial.println();

    Serial.println("System Ready.");
}

// =====================================================================================
// LOOP
// =====================================================================================

void loop()
{
    // ----------------------------------------------------
    // Handle HTTP Requests
    // ----------------------------------------------------

    server.handleClient();

    // ----------------------------------------------------
    // Alarm State Machine
    // ----------------------------------------------------

    if (alarmActive)
    {
        if (millis() - alarmStartTime >= ALARM_DURATION_MS)
        {
            alarmActive = false;

            digitalWrite(LED_PIN, LOW);
            digitalWrite(BUZZER_PIN, LOW);

            Serial.println();
            Serial.println("Alarm timeout reached.");
            Serial.println("Hardware returned to IDLE.");
        }
    }

    // ----------------------------------------------------
    // WiFi Health Check Every 30 Seconds
    // ----------------------------------------------------

    static unsigned long lastWiFiCheck = 0;

    if (millis() - lastWiFiCheck >= 30000)
    {
        lastWiFiCheck = millis();

        checkAndMaintainWiFi();
    }

    // ----------------------------------------------------
    // Yield CPU
    // ----------------------------------------------------

    delay(2);
}