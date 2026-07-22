#include <IRremote.hpp>

#define IR_RECEIVE_PIN 3    // Change this to the GPIO you're using

void setup() {
  Serial.begin(115200);

  // Initialize the IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  Serial.println("IR Receiver Started...");
}

void loop() {
  if (IrReceiver.decode()) {

    // Print the raw hexadecimal value
    Serial.print("Raw Data: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    // Print decoded protocol, address, command, etc.
    IrReceiver.printIRResultShort(&Serial);

    // Ready for the next IR signal
    IrReceiver.resume();

    delay(50);
  }
}