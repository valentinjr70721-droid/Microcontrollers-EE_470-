#include <Arduino.h>

/*
 * ESP8266 LiPo Battery Charging and Discharging Data Logger
 *
 *
 * Description:
 * This program uses an ESP8266 to measure the voltage of a
 * 3.7 V LiPo battery through a voltage divider. Battery voltage
 * is measured every 60 seconds and sent through the serial port
 * for discharging analysis.
 *
 * Hardware:
 * - ESP8266 NodeMCU
 * - 3.7 V 900 mAh LiPo Battery
 * - 1 kOhm / 2 kOhm Voltage Divider
 * - LiPo Charging Module
 *
 * Author: Miguel Valentin
 * Course: EE-470
 * Date: 10/05/26
 *
 */

// Voltage divider
const float R1 = 1000.0;
const float R2 = 2000.0;

// ESP8266 ADC
const float ADC_MAX = 1023.0;
const float A0_FULL_SCALE = 3.3;

// Calibration for your replacement ESP8266
const float CALIBRATION_FACTOR = 0.9262;

// Record once every 60 seconds
const unsigned long INTERVAL = 60000;

// Median filtering
const int NUM_SAMPLES = 21;

unsigned long lastMeasurement = 0;
unsigned long measurementNumber = 0;


// -----------------------------------------------------
// Take 21 ADC samples and return median
// -----------------------------------------------------

int readMedianADC() {

  int samples[NUM_SAMPLES];

  for (int i = 0; i < NUM_SAMPLES; i++) {
    samples[i] = analogRead(A0);
    delay(10);
  }

  // Sort samples
  for (int i = 0; i < NUM_SAMPLES - 1; i++) {

    for (int j = i + 1; j < NUM_SAMPLES; j++) {

      if (samples[j] < samples[i]) {

        int temp = samples[i];
        samples[i] = samples[j];
        samples[j] = temp;
      }
    }
  }

  return samples[NUM_SAMPLES / 2];
}


// -----------------------------------------------------
// Take one battery measurement
// -----------------------------------------------------

void takeMeasurement() {

  int adcValue = readMedianADC();

  // Voltage at ESP8266 A0
  float voltageA0 =
      adcValue * (A0_FULL_SCALE / ADC_MAX);

  // Calculate actual battery voltage
  float batteryVoltage =
      voltageA0 *
      ((R1 + R2) / R2) *
      CALIBRATION_FACTOR;

  // Time since experiment started
  float timeMinutes =
      millis() / 60000.0;

  measurementNumber++;

  // CSV output
  Serial.print(measurementNumber);
  Serial.print(",");

  Serial.print(timeMinutes, 1);
  Serial.print(",");

  Serial.print(adcValue);
  Serial.print(",");

  Serial.println(batteryVoltage, 3);
}


// -----------------------------------------------------
// Setup
// -----------------------------------------------------

void setup() {

  Serial.begin(9600);

  delay(2000);

  Serial.println();
  Serial.println("LiPo DISCHARGE TEST");

  Serial.println(
    "Measurement,Time(min),ADC,BatteryVoltage(V)"
  );

  // First measurement
  takeMeasurement();

  lastMeasurement = millis();
}


// -----------------------------------------------------
// Main loop
// -----------------------------------------------------

void loop() {

  if (millis() - lastMeasurement >= INTERVAL) {

    lastMeasurement += INTERVAL;

    takeMeasurement();
  }
}
