#include <HX711-multi.h>
#include <EEPROM.h>

// Pins
#define CLK A0 // Common clock pin for all HX711 units
#define DOUT1 A1
#define DOUT2 A2
#define DOUT3 A3
#define DOUT4 A4

// Array of data pins
byte DOUTS[5] = {DOUT1, DOUT2, DOUT3, DOUT4};

// Calibration factors for each load cell
float calibrationFactors[4] = {1.0, 1.0, 1.0, 1.0};
const int calibrationEEPROMAddress = 0;  // Starting EEPROM address for calibration factors

// Number of load cells
#define CHANNEL_COUNT sizeof(DOUTS) / sizeof(byte)

// Variables
HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);
long results[CHANNEL_COUNT];
bool tareComplete = false;

void setup() {
  Serial.begin(57600);
  Serial.println("Starting...");

  tare();  // Tare each load cell before calibration
  Serial.println("Tare complete.");

  calibrate();  // Start calibration process
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'r') calibrate();
    if (command == 't') tare();
  }
}

void tare() {
  Serial.println("Taring...");
  while (!scales.tare(20, 10000));  // Tare all load cells
  Serial.println("Tare complete.");
  tareComplete = true;
}

// Calibrate each load cell individually
void calibrate() {
  Serial.println("Starting calibration...");

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Serial.print("Place known weight on load cell ");
    Serial.print(i + 1);
    Serial.println(" and enter the weight (e.g., 100.0): ");

    // Wait for known weight input
    float knownMass = 0;
    while (Serial.available() == 0) {}
    knownMass = Serial.parseFloat();
    Serial.print("Known weight: ");
    Serial.println(knownMass);

    // Measure raw data with known weight
    scales.read(results);
    long rawValue = results[i];
    calibrationFactors[i] = rawValue / knownMass;  // Calculate calibration factor

    Serial.print("Calibration factor for load cell ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(calibrationFactors[i]);
    
    // Save calibration factor to EEPROM
    int address = calibrationEEPROMAddress + (i * sizeof(float));
    EEPROM.put(address, calibrationFactors[i]);
    delay(500);
  }

  Serial.println("Calibration complete.");
}

void loadCalibrationFromEEPROM() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    int address = calibrationEEPROMAddress + (i * sizeof(float));
    EEPROM.get(address, calibrationFactors[i]);
    Serial.print("Loaded calibration factor for load cell ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(calibrationFactors[i]);
  }
}
