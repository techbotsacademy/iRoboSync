/* MODIFIED CODE: Pointing Action for ALL targets
  - 'L' and 'P' will now point to the LEFT (135 degrees)
  - 'R' and 'Q' will now point to the RIGHT (-45 degrees)
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

const int BASE_POINT_LEFT   = 135;  // Left point
const int BASE_POINT_RIGHT  = -45;  // Right point

void setup() {
  Serial.begin(9600);
  baseServo.attach(9);
  shoulderServo.attach(6);
  elbowServo.attach(5);
  clawServo.attach(3);
  goHome();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      // Both Person and Objects now use the same pointing logic
      case 'L':
      case 'P':
        pointTo(BASE_POINT_LEFT);
        break;

      case 'R':
      case 'Q':
        pointTo(BASE_POINT_RIGHT);
        break;

      case 'S':
        goHome();
        break;
    }
  }
}

void goHome() {
  baseServo.write(BASE_HOME);
  shoulderServo.write(SHOULDER_HOME);
  elbowServo.write(ELBOW_HOME);
  clawServo.write(CLAW_HOME);
}

void pointTo(int baseAngle) {
  // Only the base moves. Shoulder, Elbow, and Claw stay at home.
  baseServo.write(baseAngle);
}