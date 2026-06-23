#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

Servo s1, s2, s3, s4;

typedef struct struct_message {
    int angle1;
    int angle2;
    int angle3;
    int angle4;
} struct_message;

struct_message incomingData;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingBytes, int len) {
  memcpy(&incomingData, incomingBytes, sizeof(incomingData));
  
  s1.write(incomingData.angle1);
  s2.write(incomingData.angle2);
  s3.write(incomingData.angle3);
  s4.write(incomingData.angle4);
  
  Serial.printf("Recv: %d, %d, %d, %d\n", incomingData.angle1, incomingData.angle2, incomingData.angle3, incomingData.angle4);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Servo Pins: 13, 14, 12, 27
  s1.attach(13);
  s2.attach(14);
  s3.attach(12);
  s4.attach(27);

  if (esp_now_init() != ESP_OK) return;
  
  esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);
}

void loop() {}


