/*
  ROBOTIC ARM - VOICE CONTROLLED (via Web Serial)
  =================================================
  Pin Connections:
    Pin 9 -> BASE servo     (left / right rotation)
    Pin 6 -> SHOULDER servo (up / down lean)
    Pin 5 -> ELBOW servo    (forward / back reach)
    Pin 3 -> CLAW servo     (open / close)

  HOME / START ANGLES (set at power-on, arm seedha khada rehta hai):
    BASE     = 90  (center)
    SHOULDER = 90  (upright)
    ELBOW    = 90  (straight)
    CLAW     = 90  (neutral/slightly open, ready to grab)

  COMMANDS RECEIVED FROM JAVASCRIPT (Web Serial), ek character bhejega:
    'L' -> Sirf POINT karo LEFT taraf (person ya object ki taraf dekhna, claw nahi khulta)
           Used for: "find person" (jab person LEFT box me ho)
    'R' -> Sirf POINT karo RIGHT taraf (person ya object ki taraf dekhna, claw nahi khulta)
           Used for: "find person" (jab person RIGHT box me ho)
    'P' -> PICK sequence LEFT taraf (object ke liye - base ghoome + shoulder/elbow jhuke + claw band/open)
    'Q' -> PICK sequence RIGHT taraf (object ke liye - same as 'P' but right side)
    'S' -> STOP / go back to HOME position (sab servo neutral pe)

  NOTE: 'L' aur 'R' ka use sirf "point out person" ke liye hota hai (JS code me).
        'P' aur 'Q' ka use object pick karne ("find wheel", "touch wheel" etc) ke liye hota hai.
*/

#include <Servo.h>

Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo clawServo;

// ---------- HOME / NEUTRAL ANGLES ----------
const int BASE_HOME     = 90;
const int SHOULDER_HOME = 90;
const int ELBOW_HOME    = 90;
const int CLAW_HOME     = 90;   // neutral / slightly open

// ---------- POINTING ANGLES (sirf dekhna, pakadna nahi) ----------
const int BASE_POINT_LEFT   = 135;   // left taraf point
const int BASE_POINT_RIGHT  = -45;  // right taraf point

// ---------- PICK SEQUENCE ANGLES (object uthana) ----------
const int BASE_PICK_LEFT    = 150;   // left box ki taraf base ghoomega
const int BASE_PICK_RIGHT   = -30;  // right box ki taraf base ghoomega
const int SHOULDER_DOWN     = 150;  // shoulder jhukega object ki taraf
const int ELBOW_REACH       = 40;   // elbow aage reach karega
const int CLAW_OPEN         = 30;   // claw fully open (pakadne se pehle)
const int CLAW_CLOSE        = 130;  // claw band (object ko pakad liya)

void setup() {
  Serial.begin(9600);

  baseServo.attach(9);
  shoulderServo.attach(6);
  elbowServo.attach(5);
  clawServo.attach(3);

  goHome();   // power on hote hi sab servo neutral/home angle pe set ho jate hain
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'L':
        pointTo(BASE_POINT_LEFT);
        break;

      case 'R':
        pointTo(BASE_POINT_RIGHT);
        break;

      case 'P':
        pickSequence(BASE_PICK_LEFT);
        break;

      case 'Q':
        pickSequence(BASE_PICK_RIGHT);
        break;

      case 'S':
        goHome();
        break;
    }
  }
}

// ---------------- HOME ----------------
void goHome() {
  baseServo.write(BASE_HOME);
  delay(300);
  shoulderServo.write(SHOULDER_HOME);
  delay(300);
  elbowServo.write(ELBOW_HOME);
  delay(300);
  clawServo.write(CLAW_HOME);
  delay(300);
}

// ---------------- POINT (sirf base ghoomega, claw kabhi nahi khulega) ----------------
void pointTo(int baseAngle) {
  baseServo.write(baseAngle);
  delay(500);
  // Shoulder, elbow, claw apni HOME position pe hi rehte hain.
  // Kuch der baad apne aap wapas home (JS side se 'S' command aata hai jab box empty ho jaata hai)
}

// ---------------- PICK SEQUENCE (object pakadna) ----------------
void pickSequence(int baseAngle) {
  // Step 1: Base us taraf ghoomo jahan object hai
  baseServo.write(baseAngle);
  delay(600);

  // Step 2: Shoulder + Elbow jhuko object ki taraf, claw khulo
  shoulderServo.write(SHOULDER_DOWN);
  delay(500);
  elbowServo.write(ELBOW_REACH);
  delay(500);
  clawServo.write(CLAW_OPEN);
  delay(500);

  // Step 3: Claw band karo (object pakad liya)
  clawServo.write(CLAW_CLOSE);
  delay(2000);   // <-- 2 second wait jaise aapne bola

  // Step 4: Arm uthao (object pakad ke upar le aao)
  shoulderServo.write(SHOULDER_HOME);
  delay(500);
  elbowServo.write(ELBOW_HOME);
  delay(500);

  // Step 5: Pehle ki (HOME) position pe wapas aao
  baseServo.write(BASE_HOME);
  delay(600);
  clawServo.write(CLAW_HOME);
  delay(300);
}