// Pins
const int TOUCH_PIN = 4; // D4 pin for Touch
const int LED_PIN   = 2; // D2 pin for LED

// Touch sensitivity threshold
const int TOUCH_THRESHOLD = 500;

bool ledState = false;
bool isTouched = false;

void setup() {
  Serial.begin(115200);

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Read touch value from D4 wire
  int touchValue = touchRead(TOUCH_PIN);

  // Print touch reading to Serial Monitor
  Serial.print("Touch Reading: ");
  Serial.println(touchValue);

  // Check if touched (Value drops when finger touches the wire)
  if (touchValue < TOUCH_THRESHOLD && !isTouched) {
    ledState = !ledState; // Toggle LED ON/OFF
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);

    isTouched = true; // Prevent rapid toggling while holding
    delay(300);       // Debounce delay
  }

  // Reset when finger is removed
  if (touchValue >= TOUCH_THRESHOLD) {
    isTouched = false;
  }

  delay(100);
}