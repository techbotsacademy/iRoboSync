#include <Servo.h>

Servo elbowServo;

const int elbowPin = 9;   // Arduino Uno servo pin

int currentAngle = 90;
int targetAngle = 90;

void setup() {
  Serial.begin(115200);

  elbowServo.attach(elbowPin);
  elbowServo.write(currentAngle);

  Serial.println("Arduino Ready");
}

void loop() {

  // Receive Y value from HTML over Web Serial
  if (Serial.available()) {

    int y = Serial.parseInt();

    if (y >= 0 && y <= 380) {

      targetAngle = map(y, 0, 380, 10, 125);
      targetAngle = constrain(targetAngle, 10, 125);

      Serial.print("Y: ");
      Serial.print(y);
      Serial.print("  Servo: ");
      Serial.println(targetAngle);
    }

    // Clear remaining characters (newline, etc.)
    while (Serial.available()) {
      Serial.read();
    }
  }

  // Smooth servo movement
  if (currentAngle < targetAngle) {
    currentAngle++;
    elbowServo.write(currentAngle);
    delay(10);
  }
  else if (currentAngle > targetAngle) {
    currentAngle--;
    elbowServo.write(currentAngle);
    delay(10);
  }
}