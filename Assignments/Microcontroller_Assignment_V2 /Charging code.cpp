#include <Arduino.h>

/*
 * ESP8266 LiPo Battery Charging and Discharging Data Logger
 *
 * Description:
 * This program uses an ESP8266 to measure the voltage of a
 * 3.7 V LiPo battery through a voltage divider. Battery voltage
 * is measured every 60 seconds and sent through the serial port
 * for charging analysis.
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
 * Version: 1 
 *
 */



// ---------- Voltage Divider ----------
const float R1 = 1000.0;   // 1 kOhm
const float R2 = 2000.0;   // 2 kOhm




// ---------- ESP8266 ADC ----------
const float ADC_MAX = 1023.0;
const float A0_FULL_SCALE = 3.3;




// ---------- CALIBRATION ----------
//
// New ESP8266:
// Multimeter = 3.926 V
// ESP8266 before calibration = 4.239 V
//
// 3.926 / 4.239 = approximately 0.9262
//
const float CALIBRATION_FACTOR = 0.9262;




// ---------- Timing ----------


// 60,000 ms = 60 seconds = 1 minute
const unsigned long MEASUREMENT_INTERVAL = 60000;




// ---------- ADC Filtering ----------
//
// Take 21 quick measurements.
// The median rejects occasional bad readings
// such as ADC = 12.
//
const int NUM_SAMPLES = 21;




unsigned long lastMeasurementTime = 0;
unsigned long measurementNumber = 0;




// =====================================================
// MEDIAN ADC FUNCTION
// =====================================================


int readMedianADC() {


  int samples[NUM_SAMPLES];


  // Take 21 quick ADC readings
  for (int i = 0; i < NUM_SAMPLES; i++) {


    samples[i] = analogRead(A0);


    // Small delay between ADC readings
    delay(10);
  }




  // Sort readings from smallest to largest
  for (int i = 0; i < NUM_SAMPLES - 1; i++) {


    for (int j = i + 1; j < NUM_SAMPLES; j++) {


      if (samples[j] < samples[i]) {


        int temp = samples[i];


        samples[i] = samples[j];


        samples[j] = temp;
      }
    }
  }




  // With 21 samples, sample #11 is the middle value
  return samples[NUM_SAMPLES / 2];
}




// =====================================================
// BATTERY MEASUREMENT FUNCTION
// =====================================================


void takeMeasurement() {


  // Get filtered ADC value
  int adcValue = readMedianADC();




  // Convert ADC number to voltage at A0
  float voltageA0 =
      adcValue *
      (A0_FULL_SCALE / ADC_MAX);




  // Undo the external voltage divider
  //
  // Vbattery = VA0 × (R1 + R2) / R2
  //
  float batteryVoltage =
      voltageA0 *
      ((R1 + R2) / R2);




  // Apply calibration for NEW ESP8266
  batteryVoltage =
      batteryVoltage *
      CALIBRATION_FACTOR;




  // Calculate elapsed time
  float elapsedMinutes =
      millis() / 60000.0;




  // Increase measurement number
  measurementNumber++;




  // ===================================================
  // CSV OUTPUT
  // ===================================================


  Serial.print(measurementNumber);


  Serial.print(",");


  Serial.print(elapsedMinutes, 1);


  Serial.print(",");


  Serial.print(adcValue);


  Serial.print(",");


  Serial.println(batteryVoltage, 3);
}




// =====================================================
// SETUP
// =====================================================


void setup() {


  Serial.begin(9600);


  // Give serial connection time to start
  delay(2000);




  Serial.println();


  Serial.println(
      "Measurement,Time(min),ADC,BatteryVoltage(V)"
  );




  // Take measurement #1 immediately
  takeMeasurement();




  // Start 60-second timer
  lastMeasurementTime = millis();
}




// =====================================================
// MAIN LOOP
// =====================================================


void loop() {


  // Check whether 60 seconds have passed
  if (millis() - lastMeasurementTime
      >= MEASUREMENT_INTERVAL) {




    // Maintain the 60-second schedule
    lastMeasurementTime +=
        MEASUREMENT_INTERVAL;




    // Record battery voltage
    takeMeasurement();
  }
}














