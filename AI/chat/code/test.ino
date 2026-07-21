#include <LittleFS.h>

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  Serial.println("LittleFS Mounted!");
}

void loop() {
}