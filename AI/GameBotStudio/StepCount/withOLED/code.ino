#include <Wire.h>
#include <MPU6050.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

// TFT Pins
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
#define TFT_MOSI  23
#define TFT_SCLK  18

Adafruit_GC9A01A display(
  TFT_CS,
  TFT_DC,
  TFT_MOSI,
  TFT_SCLK,
  TFT_RST
);

// MPU6050
MPU6050 mpu;

// Step Counter
long stepCount = 0;

float lastMagnitude = 0;

unsigned long lastStepTime = 0;

const float STEP_THRESHOLD = 2500.0;
const unsigned long STEP_DELAY = 300;

void drawScreen()
{
  display.fillScreen(GC9A01A_BLACK);

  display.setTextColor(GC9A01A_CYAN);

  display.setTextSize(2);
  display.setCursor(55, 30);
  display.print("AI");

  display.setCursor(25, 55);
  display.print("FITNESS");

  display.drawCircle(
    120,
    120,
    70,
    GC9A01A_BLUE
  );

  display.setTextColor(GC9A01A_WHITE);

  display.setTextSize(4);

  display.setCursor(70, 100);
  display.print(stepCount);

  display.setTextSize(2);

  display.setCursor(60, 160);
  display.print("STEPS");
}

void setup()
{
  Serial.begin(115200);

  // MPU6050
  Wire.begin(21,22);

  mpu.initialize();

  // TFT
  display.begin();

  display.setRotation(0);

  display.fillScreen(GC9A01A_BLACK);

  display.setTextColor(GC9A01A_GREEN);

  display.setTextSize(2);

  display.setCursor(35,110);
  display.print("START");

  delay(2000);

  drawScreen();
}

void loop()
{
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(
    &ax,
    &ay,
    &az,
    &gx,
    &gy,
    &gz
  );

  float magnitude =
    sqrt(
      (float)ax * ax +
      (float)ay * ay +
      (float)az * az
    );

  float change =
    fabs(
      magnitude -
      lastMagnitude
    );

  if(change > STEP_THRESHOLD)
  {
    if(
      millis() -
      lastStepTime >
      STEP_DELAY
    )
    {
      stepCount++;

      lastStepTime = millis();

      Serial.print("Steps: ");
      Serial.println(stepCount);

      drawScreen();
    }
  }

  lastMagnitude = magnitude;

  delay(20);
}

