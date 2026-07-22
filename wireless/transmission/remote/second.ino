#include <IRremote.hpp>

#define IR_RECEIVE_PIN 3

enum TrainingState {
  TRAIN_UP,
  TRAIN_DOWN,
  TRAIN_LEFT,
  TRAIN_RIGHT,
  TRAIN_COMPLETE
};

TrainingState state = TRAIN_UP;

uint32_t upCode = 0;
uint32_t downCode = 0;
uint32_t leftCode = 0;
uint32_t rightCode = 0;

void printCopyBlock() {

  Serial.println();
  Serial.println("==================================");
  Serial.println("COPY THE BELOW INTO THE HTML PAGE");
  Serial.println("==================================");

  Serial.print("UP=");
  Serial.println(upCode, HEX);

  Serial.print("DOWN=");
  Serial.println(downCode, HEX);

  Serial.print("LEFT=");
  Serial.println(leftCode, HEX);

  Serial.print("RIGHT=");
  Serial.println(rightCode, HEX);

  Serial.println("==================================");
}

void setup() {

  Serial.begin(115200);

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  Serial.println();
  Serial.println("ESP32 IR Remote Trainer");
  Serial.println("-----------------------");
  Serial.println();
  Serial.println("Press the button you want to use for UP...");
}

void loop() {

  if (!IrReceiver.decode())
    return;

  uint32_t code = IrReceiver.decodedIRData.decodedRawData;

  switch (state) {

    case TRAIN_UP:

      upCode = code;

      Serial.print("Captured : 0x");
      Serial.println(code, HEX);

      Serial.println("Mapped To : UP");
      Serial.println();
      Serial.println("Press the button you want to use for DOWN...");

      state = TRAIN_DOWN;
      break;

    case TRAIN_DOWN:

      downCode = code;

      Serial.print("Captured : 0x");
      Serial.println(code, HEX);

      Serial.println("Mapped To : DOWN");
      Serial.println();
      Serial.println("Press the button you want to use for LEFT...");

      state = TRAIN_LEFT;
      break;

    case TRAIN_LEFT:

      leftCode = code;

      Serial.print("Captured : 0x");
      Serial.println(code, HEX);

      Serial.println("Mapped To : LEFT");
      Serial.println();
      Serial.println("Press the button you want to use for RIGHT...");

      state = TRAIN_RIGHT;
      break;

    case TRAIN_RIGHT:

      rightCode = code;

      Serial.print("Captured : 0x");
      Serial.println(code, HEX);

      Serial.println("Mapped To : RIGHT");

      state = TRAIN_COMPLETE;

      printCopyBlock();

      break;

    case TRAIN_COMPLETE:
      break;
  }

  IrReceiver.resume();
  delay(300);
}