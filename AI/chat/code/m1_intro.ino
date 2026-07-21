void setup() {
  Serial.begin(115200);

  pinMode(2, OUTPUT);   // Built-in LED (Most ESP32 DevKit V1 boards)
  digitalWrite(2, LOW);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == '1') {
      digitalWrite(2, HIGH);
      Serial.println("LED ON");
    }

    if (c == '0') {
      digitalWrite(2, LOW);
      Serial.println("LED OFF");
    }
  }
}