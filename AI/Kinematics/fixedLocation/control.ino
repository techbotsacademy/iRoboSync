Servo base, shoulder, elbow, claw;

int posBase = 90, posShoulder = 90; 


//void clawAction() {

//  // Munh kholna

//  for (int i = 0; i <= 90; i++) { claw.write(i); delay(10); }

//  delay(400); 

//  // Munh band karna

//  for (int i = 90; i >= 0; i--) { claw.write(i); delay(10); }

//}


void smoothMove(Servo &servo, int &currentPos, int target) {

  if (currentPos == target) return;

  if (currentPos < target) {

    for (int i = currentPos; i <= target; i++) { servo.write(i); delay(15); }

  } else {

    for (int i = currentPos; i >= target; i--) { servo.write(i); delay(15); }

  }

  currentPos = target;

}


void setup() {

  Serial.begin(9600);

  base.attach(3); shoulder.attach(5); elbow.attach(6); claw.attach(9);

  

  // Starting position setup

  base.write(90); shoulder.write(90); elbow.write(90); claw.write(0);

}


void loop() {

  if (Serial.available() > 0) {

    char command = Serial.read();

    

    if (command == 'L') {

      smoothMove(base, posBase, 135); 

      smoothMove(shoulder, posShoulder, 45); 

      delay(200);

      claw.write(90); 

    } 

    else if (command == 'R') {

      smoothMove(base, posBase, 45); 

      smoothMove(shoulder, posShoulder, 45); 

      delay(200);

      claw.write(90);

    } 

    else if (command == 'S') { 

      // Home aana aur claw action karna

      smoothMove(shoulder, posShoulder, 90); 

      smoothMove(base, posBase, 90); 

      delay(200);

      claw.write(0); 

    }

  }

}

