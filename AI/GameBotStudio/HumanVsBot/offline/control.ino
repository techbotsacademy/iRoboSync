#include <Servo.h>

Servo elbowServo;

const int elbowPin = 9;   // Arduino Uno servo pin

int currentAngle = 90;
int targetAngle = 90;

void setup() {
  Serial.begin(9600);
  elbowServo.attach(elbowPin);
  elbowServo.write(currentAngle);

  Serial.println("Arduino Ready");
}

void loop() {

  // Receive Y value from HTML over Web Serial
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // Read full line
    int y = input.toInt(); // Convert to number 

    if (y >= 0 && y <= 380) {

      targetAngle = map(y, 0, 380, 10, 150);
      targetAngle = targetAngle;
    }
    Serial.print("Target Y: ");
    Serial.print(y);
    Serial.print("  Servo Target: ");
    Serial.println(targetAngle);

     if (targetAngle > currentAngle) {
        // Move forward
        for (int angle = currentAngle; angle <= targetAngle; angle += 1) {
          elbowServo.write(angle);
          delay(5);
        }
      } else {
        // Move backward
        for (int angle = currentAngle; angle >= targetAngle; angle -= 1) {
          elbowServo.write(angle);
          delay(5);
        }
      }

      // Ensure the servo reaches the exact target
      elbowServo.write(targetAngle);
      currentAngle = targetAngle;
  }

}