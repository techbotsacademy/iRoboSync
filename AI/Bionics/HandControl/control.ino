#include <Servo.h>

// Servo setup
Servo servo1, servo2, servo3, servo4;

void setup() {
  Serial.begin(115200); // Fast communication
  
  // Apne pins yahan set karein
  servo1.attach(9);  // Index
  servo2.attach(10); // Middle
  servo3.attach(11); // Ring
  servo4.attach(12); // Pinky

  // Initial Position: Hand Open (0 deg)
  servo1.write(0);
  servo2.write(0);
  servo3.write(0);
  servo4.write(0);
}

void loop() {
  if (Serial.available() > 0) {
    // Read numbers from string (e.g., "180,90,45,0")
    int val1 = Serial.parseInt();
    int val2 = Serial.parseInt();
    int val3 = Serial.parseInt();
    int val4 = Serial.parseInt();

    // Jab newline '\n' mile tab execute karein
    if (Serial.read() == '\n') {
      servo1.write(val1);
      servo2.write(val2);
      servo3.write(val3);
      servo4.write(val4);
    }
  }
}
