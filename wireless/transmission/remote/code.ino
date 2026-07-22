#include <IRremote.hpp>

#define IR_RECEIVE_PIN 3

enum TrainingState {
  TRAIN_UP,
  TRAIN_DOWN,
  TRAIN_LEFT,
  TRAIN_RIGHT,
  TRAIN_COMPLETE
};

TrainingState trainingState = TRAIN_UP;

uint32_t upCode = 0;
uint32_t downCode = 0;
uint32_t leftCode = 0;
uint32_t rightCode = 0;

void setup() {
  Serial.begin(115200);

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("ESP32 IR Remote Trainer");
  Serial.println("=========================================");
  Serial.println();
  Serial.println("Press the button you want to use for UP...");
}

void loop() {

  if (!IrReceiver.decode())
    return;

  uint32_t code = IrReceiver.decodedIRData.decodedRawData;

  switch (trainingState) {

    case TRAIN_UP:

      upCode = code;

      Serial.print("Captured Code : 0x");
      Serial.println(upCode, HEX);
      Serial.println("Mapped Action : UP");
      Serial.println();
      Serial.println("Press the button you want to use for DOWN...");

      trainingState = TRAIN_DOWN;
      break;

    case TRAIN_DOWN:

      downCode = code;

      Serial.print("Captured Code : 0x");
      Serial.println(downCode, HEX);
      Serial.println("Mapped Action : DOWN");
      Serial.println();
      Serial.println("Press the button you want to use for LEFT...");

      trainingState = TRAIN_LEFT;
      break;

    case TRAIN_LEFT:

      leftCode = code;

      Serial.print("Captured Code : 0x");
      Serial.println(leftCode, HEX);
      Serial.println("Mapped Action : LEFT");
      Serial.println();
      Serial.println("Press the button you want to use for RIGHT...");

      trainingState = TRAIN_RIGHT;
      break;

    case TRAIN_RIGHT:

      rightCode = code;

      Serial.print("Captured Code : 0x");
      Serial.println(rightCode, HEX);
      Serial.println("Mapped Action : RIGHT");

      trainingState = TRAIN_COMPLETE;

      Serial.println();
      Serial.println("=========================================");
      Serial.println("Training Complete");
      Serial.println("=========================================");

      Serial.print("UP    : 0x");
      Serial.println(upCode, HEX);

      Serial.print("DOWN  : 0x");
      Serial.println(downCode, HEX);

      Serial.print("LEFT  : 0x");
      Serial.println(leftCode, HEX);

      Serial.print("RIGHT : 0x");
      Serial.println(rightCode, HEX);

      Serial.println();
      Serial.println("Remote is now ready.");
      Serial.println("Press any trained button...");
      break;

    case TRAIN_COMPLETE:

      Serial.print("Captured Code : 0x");
      Serial.println(code, HEX);

      if (code == upCode)
        Serial.println("Action : UP");
      else if (code == downCode)
        Serial.println("Action : DOWN");
      else if (code == leftCode)
        Serial.println("Action : LEFT");
      else if (code == rightCode)
        Serial.println("Action : RIGHT");
      else
        Serial.println("Action : UNKNOWN");

      Serial.println();
      break;
  }

  IrReceiver.resume();
  delay(150);   // Prevent duplicate reads from a long button press
}