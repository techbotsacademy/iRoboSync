// ESP32 Touch Test
// Just test touch pin - Touch2 is T2 which is on GPIO 2.

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);  // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");
  pinMode(2,OUTPUT);
}

void loop() {
  Serial.println(touchRead(4));  // get value using T2
  delay(1000);
  if(touchRead(4)<1200){
    digitalWrite(2, HIGH);
  }
  else
  {
    digitalWrite(2,LOW);
  }
}
