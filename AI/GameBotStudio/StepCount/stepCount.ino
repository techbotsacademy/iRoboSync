#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

long stepCount = 0;

// Step detection parameters
float threshold = 1.15;          // Tune this
unsigned long lastStepTime = 0;
const unsigned long minStepTime = 300;  // ms

// Gravity estimate
float gravity = 9.81;

// Low-pass filter coefficient
const float alpha = 0.90;

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }

  Serial.println("MPU6050 found.");

  // Accelerometer range
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);

  // Bandwidth
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(1000);
}

void loop() {

  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate total acceleration magnitude
  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;

  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  // Estimate gravity using low-pass filtering
  gravity = alpha * gravity + (1.0 - alpha) * magnitude;

  // Remove gravity
  float dynamicAcceleration = magnitude - gravity;

  // Absolute value
  dynamicAcceleration = abs(dynamicAcceleration);

  unsigned long now = millis();

  // Detect a step
  if (dynamicAcceleration > threshold &&
      now - lastStepTime > minStepTime) {

    stepCount++;
    lastStepTime = now;

    Serial.print("STEP!  Count = ");
    Serial.println(stepCount);
  }

  delay(20);  // ~50 Hz sampling
}