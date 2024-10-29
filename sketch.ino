#include "HX711-multi.h"
#include <EEPROM.h>
// Include math library for isnan() to recalibrate load cells with no calibration values in EEPROM
// And if user send not a number from serial monitor
#include <math.h>

// Pins
#define CLK 2 // Common clock pin for all HX711 units
#define DOUT1 3
#define DOUT2 4
#define DOUT3 5
#define DOUT4 6
#define DOUT5 7
#define DOUT6 8
#define DOUT7 9
#define DOUT8 10
// #define DOUT5 A5

#define TARE_TIMEOUT_SECONDS 4

// Array of data pins
byte DOUTS[8] = {DOUT1, DOUT2, DOUT3, DOUT4, DOUT5, DOUT6, DOUT7, DOUT8};

// Calibration values for each load cell
// float calibrationValues[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
float calibrationValues[8] = {0.42, 0.42, 0.42, 0.42, 0.42, 0.42, 0.42, 0.42};
const int calibrationEEPROMAddress = 0;  // Starting EEPROM address for calibration values

// Number of load cells
#define CHANNEL_COUNT sizeof(DOUTS) / sizeof(byte)

// Initialize HX711 units
// define clock and data pin, channel, and gain value
// channel selection is made by passing the appropriate gain: 128 or 64 for channel A, 32 for channel B
// count: the number of channels
// dout: an array of pin numbers, of length 'count', one entry per channel
// pd_sck: clock pin
// gain: 
//  case 128:	GAIN = 1;	// channel A, gain value 128
//  case 64: GAIN = 3; // channel A, gain value 64
//  case 32: GAIN = 2; // channel B, gain value 32
// HX711MULTI(int count, byte *dout, byte pd_sck, byte gain = 128);
HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);
long results[CHANNEL_COUNT];
int i = 0;

void setup() {
  Serial.begin(57600);
  Serial.println("Starting setuping...");
  // Serial.flush();
  // pinMode(11, OUTPUT);  // For some reason, if you add this pin, the console starts to display the text

  tare();

  // Ask if you want to calibrate, use values from memory, or use default values
  Serial.println("***");
  Serial.println("To calibrate load cells, send 'c' from serial monitor");
  Serial.println("To use load cells calibration values from EEPROM adress, send 'm' from serial monitor");
  Serial.println("To use default load cells calibration values, send 'd' from serial monitor");

  bool _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char command = Serial.read();
      switch (command) {
        case 'c':
          calibrate(); // Start calibration process
          _resume = true;
          break;
        case 'm':
          loadCalibrationFromEEPROM();
          _resume = true;
          break;
        case 'd':
          _resume = true;
      }
    }
  }
  Serial.println("Finishing setuping...");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'c' from serial monitor.");
  Serial.println("To set the tare offsets, send 't' from serial monitor.");
}

void loop() {
  Serial.print("Number of read:");
  Serial.print(i);
  Serial.println();
  sendRawData(); //this is for sending raw data, for where everything else is done in processing
  i++;
  delay(1000);

  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 't') tare(); //tare
    else if (command == 'c') calibrate(); //calibrate
    // else if (command == 'e') changeSavedCalFactor(); //edit calibration value manually
  }
}

void sendRawData() {
  // Serial.println("Before");
  scales.read(results);
  // Serial.println("After");
  for (int i=0; i < scales.get_count(); ++i) {
    // Serial.println("Before");
    Serial.print(i+1);
    Serial.print(":");
    Serial.print(results[i] / calibrationValues[i]);
    Serial.print("  ");
    // Serial.println("After");
    // Serial.print( (i!=scales.get_count()-1)?"\t":"\n");
  }
  // Serial.println("After 2");
  Serial.println();
}

void tare() {
  Serial.println("***");
  Serial.println("Place the load cells an a level stable surface.");
  Serial.println("Remove any load applied to the load cells.");
  Serial.println("To set the tare offset, send 't' from serial monitor.");
  Serial.println("To skip setting tare offset, send 's' from serial monitor.");
  Serial.println("Current data of load cells:");
  sendRawData();

  bool _resume = false;
  char command;
  while(_resume == false){
    while(!Serial.available());
    command = Serial.read();
    if (command == 't') {
      _resume = true;
    } 
    else if (command == 's'){
      Serial.println("Skipping setting tare offset.");
      return;
    }
  }

  Serial.println("Taring...");
  bool tareSuccessful = false;
  // Tare all load cells
  unsigned long tareStartTime = millis();
  while (!tareSuccessful && millis() < (tareStartTime + TARE_TIMEOUT_SECONDS*1000)) {
    // Serial.println("Before");
    tareSuccessful = scales.tare(20, 10000);  //reject 'tare' if still ringing
    // Serial.println("After");
  }
  if (!tareSuccessful){
    Serial.println("Timeout, check wiring and pin designations");
    while (1);
  }
  Serial.println("Current data of load cells:");
  sendRawData();
  Serial.println("Tare complete.");
}

// Calibrate each load cell individually
void calibrate() {
  // loadCalibrationFromEEPROM();
  // for (int i = 0; i < CHANNEL_COUNT; i++) {
  //   if (isnan(calibrationValues[i])){
  //     Serial.println("ABOBA");
  //   }
  // }
  Serial.println("***");
  Serial.println("Start calibration:");

  tare();

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Serial.print("Calibrating ");
    Serial.print(i + 1);
    Serial.print(" load cell. To skip calibrating of this load cell and use previous calibrating value, send 's'. To continue, send any");

    char command;
    while (!Serial.available());
    command = Serial.read();
    if (command == 's') continue;

    Serial.print("Place known weight on load cell ");
    Serial.print(i + 1);
    Serial.println(" and enter the weight (in grams)(e.g., 100.0): ");

    // Wait for known weight input
    float knownMass = 0;
    boolean _resume = false;
    while (_resume == false) {
      // scales.read(results);
      while(!Serial.available());
      
      knownMass = Serial.parseFloat();
      if (knownMass != 0){
        Serial.print("Known weight: ");
        Serial.println(knownMass);

        Serial.println("Continue(y/n):");
        while (!Serial.available());
        command = Serial.read();
        if (command == 'y'){
          _resume = true;
        } else {
          Serial.print("Place known weight on load cell ");
          Serial.print(i + 1);
          Serial.println(" and enter the weight (in grams)(e.g., 100.0): ");
        }
          // TODO: Maybe add isnan check here
      }
    }

    // Measure raw data with known weight
    scales.read(results);
    long rawValue = results[i];
    calibrationValues[i] = rawValue / knownMass;  // Calculate calibration value
    // // calibrationValues cant be 0
    if (isnan(calibrationValues[i]) || calibrationValues[i] == 0){
      calibrationValues[i] = 1;
    }

    Serial.print("Calibration value for load cell ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(calibrationValues[i]);
    
    int address = calibrationEEPROMAddress + (i * sizeof(float));
    Serial.print("Save this value to EEPROM adress ");
    Serial.print(address);
    Serial.println("? y/n");

    _resume = false;
    while (_resume == false) {
      while (!Serial.available());
      command = Serial.read();
      if (command == 'y') {
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.begin(512);
// #endif
        // Save calibration value to EEPROM
        EEPROM.put(address, calibrationValues[i]);
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.commit();
// #endif
        EEPROM.get(address, calibrationValues[i]);
        Serial.print("Value ");
        Serial.print(calibrationValues[i]);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(address);
        _resume = true;
      }
      else if (command == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End calibration");
  Serial.println("***");
  delay(500);
}

void loadCalibrationFromEEPROM() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    int address = calibrationEEPROMAddress + (i * sizeof(float));
    EEPROM.get(address, calibrationValues[i]);
    Serial.print("Loaded calibration value for load cell ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(calibrationValues[i]);
    // if (isnan(calibrationValues[i])){
    //   Serial.println("Calibrate value is nan. Reasigning");
    //   calibrationValues[i] = 1;
    //   Serial.println("New calibration value for load cell ");
    //   Serial.print(i + 1);
    //   Serial.print(": ");
    //   Serial.println(calibrationValues[i]);
    // }
  }
}