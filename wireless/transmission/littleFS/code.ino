#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

const int LED_PIN = 2;

void handleRoot()
{
    File file = LittleFS.open("/index.html", "r");

    if (!file)
    {
        server.send(404, "text/plain", "index.html not found");
        return;
    }

    server.streamFile(file, "text/html");
    file.close();
}

void handleToggle()
{
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    server.send(200, "text/plain", "LED Toggled");
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);

    if (!LittleFS.begin())
    {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/toggle", HTTP_GET, handleToggle);

    server.begin();
}

void loop()
{
    server.handleClient();
}