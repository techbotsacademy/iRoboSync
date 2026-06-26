/* Pointing Action for ALL targets
  - 'L' and 'P' will now point to the LEFT
  - 'R' and 'Q' will now point to the RIGHT
  - Shoulder, Elbow, and Claw remain at HOME position for every command.
*/

#include <Servo.h>

Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo clawServo;

// ---------- ANGLES ----------
const int BASE_HOME      = 90;
const int SHOULDER_HOME  = 90;
const int ELBOW_HOME     = 90;
const int CLAW_HOME      = 90;

const int BASE_POINT_LEFT   = 45;  // Left point
const int BASE_POINT_RIGHT  = -45;  // Right point

void setup() {
  Serial.begin(9600);
  baseServo.attach(9);
  baseServo.write(BASE_HOME);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      // Both Person and Objects now use the same pointing logic
      case 'L':
        baseServo.write(BASE_POINT_LEFT);
        break;

      case 'R':
        baseServo.write(BASE_POINT_RIGHT);
        break;

      case 'C':
        baseServo.write(BASE_HOME);
        break;
    }
  }
}