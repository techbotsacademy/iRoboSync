#include <esp_now.h>
#include <WiFi.h>

// Slave ka MAC Address
uint8_t broadcastAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; 

// Charo servos ke angles ke liye structure
typedef struct struct_message {
    int angle1;
    int angle2;
    int angle3;
    int angle4;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nSend Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) return;

  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) return;
}

void loop() {
  // 4 Pots reading from ADC1 pins
  myData.angle1 = map(analogRead(32), 0, 4095, 0, 180);
  myData.angle2 = map(analogRead(33), 0, 4095, 0, 180);
  myData.angle3 = map(analogRead(34), 0, 4095, 0, 180);
  myData.angle4 = map(analogRead(35), 0, 4095, 0, 180);

  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  Serial.printf("Angles: %d, %d, %d, %d\n", myData.angle1, myData.angle2, myData.angle3, myData.angle4);
  delay(50); 
}


