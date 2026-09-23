#include <Wire.h>
#include <MPU6500_WE.h>

// ESP32 NodeMCU I2C pins
constexpr uint8_t SDA_PIN = 19;
constexpr uint8_t SCL_PIN = 18;
constexpr uint8_t MPU6050_ADDRESS = 0x68;

constexpr uint32_t OUTPUT_INTERVAL_MS = 100;
constexpr float RAD_TO_DEG_F = 57.2957795131f;

MPU6500_WE mpu(MPU6050_ADDRESS);

uint32_t lastUpdateMs = 0;
float yawDeg = 0.0f;

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  if (!mpu.init()) {
    Serial.println("MPU6050/MPU6500 not detected. Check wiring and I2C address.");
    while (true) {
      delay(1000);
    }
  }

  // Calibrate the accelerometer and gyroscope while the sensor is still.
  Serial.println("Keep the sensor still: calibrating...");
  delay(1000);
  mpu.autoOffsets();

  mpu.setAccRange(MPU9250_ACC_RANGE_2G);
  mpu.setGyrRange(MPU9250_GYRO_RANGE_250);
  mpu.setAccDLPF(MPU9250_DLPF_6);
  mpu.setGyrDLPF(MPU9250_DLPF_6);

  lastUpdateMs = millis();
  Serial.println("Roll (deg), Pitch (deg), Yaw (deg)");
}

void loop() {
  const uint32_t nowMs = millis();
  if (nowMs - lastUpdateMs < OUTPUT_INTERVAL_MS) {
    return;
  }

  const float dt = (nowMs - lastUpdateMs) / 1000.0f;
  lastUpdateMs = nowMs;

  // getGValues() returns acceleration in g; getGyrValues() returns degrees/s.
  const xyzFloat acc = mpu.getGValues();
  const xyzFloat gyr = mpu.getGyrValues();

  // Accelerometer angles provide an absolute reference for roll and pitch.
  const float rollDeg = atan2f(acc.y, acc.z) * RAD_TO_DEG_F;
  const float pitchDeg = atan2f(-acc.x, sqrtf(acc.y * acc.y + acc.z * acc.z)) * RAD_TO_DEG_F;

  // Yaw has no absolute reference with an MPU6050/MPU6500 alone, so it is
  // integrated from the Z-axis gyroscope and will gradually drift over time.
  yawDeg += gyr.z * dt;
  if (yawDeg > 180.0f) {
    yawDeg -= 360.0f;
  } else if (yawDeg < -180.0f) {
    yawDeg += 360.0f;
  }

  Serial.print(rollDeg, 2);
  Serial.print(", ");
  Serial.print(pitchDeg, 2);
  Serial.print(", ");
  Serial.println(yawDeg, 2);
}
