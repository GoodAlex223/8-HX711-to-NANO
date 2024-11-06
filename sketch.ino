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

#define TARE_TIMEOUT_SECONDS 4

// Array of data pins
byte DOUTS[8] = {DOUT1, DOUT2, DOUT3, DOUT4, DOUT5, DOUT6, DOUT7, DOUT8};

// Calibration values for each load cell
float calibrationValues[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

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
int number_of_read = 0;

void setup() {
  Serial.begin(57600);
  Serial.println("Starting setuping...");
  delay(100);
  // Serial.flush();
  // pinMode(11, OUTPUT);  // For some reason, if you add this pin, the console starts to display the text
  currentCalibrationValues();
  tare();

  bool _resume = false;
  char command;
  while (_resume == false) {
    // Ask if you want to calibrate, use values from memory, or use default values
    Serial.println("Commands:");
    Serial.println("- Send 'c' to change calibration values of load cells");
    Serial.println("- Send 'm' to load calibration values from EEPROM");
    Serial.println("- Send 'd' to use default calibration values");
    
    while (!Serial.available());
    command = get_command();
    if (!to_continue()){
      continue;
    }

    switch (command) {
      case 'c':
        calibrate();
        _resume = true;
        break;
      case 'm':
        loadCalibrationFromEEPROM();
        _resume = true;
        break;
      case 'd':
        Serial.println("Using default calibration values.");
        currentCalibrationValues();
        _resume = true;
        break;
      default:
        break;
    }
  }
  Serial.println("Setup complete. Ready for commands.");
  Serial.println("***");
  Serial.println("Send 'c' to change calibration values of load cells.");
  Serial.println("Send 't' to set tare offset for all load cells.");
}

void loop() {
  if (Serial.available() > 0) {
    char command = get_command();
    if (to_continue()){
      if (command == 't') tare();
      else if (command == 'c') calibrate();
      // else if (command == 'e') changeSavedCalFactor(); //edit calibration value manually
    }
  }
  Serial.print("Number of read: ");
  Serial.println(number_of_read);
  number_of_read++;

  sendCalibratedData();

  delay(1000);
}

bool to_continue() {
  Serial.println("- Continue?(y/n):");
  while (!Serial.available());
  return (Serial.read() == 'y');
}

char get_command(){
  char command = Serial.read();
  Serial.print("Your input: '");
  Serial.print(command);
  Serial.println("'.");
  return command;
}

void sendCalibratedData() {
  Serial.println("Current calibrated weights:");
  // Serial.println("Before");
  scales.read(results);
  // Serial.println("After");
  for (int i=0; i < scales.get_count(); ++i) {
    // Serial.println("Before");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(results[i] / calibrationValues[i], 4);
    Serial.print("; ");
    // Serial.println("After");
    // Serial.print( (i!=scales.get_count()-1)?"\t":"\n");
  }
  // Serial.println("After 2");
  Serial.println();
}

void currentCalibrationValues() {
  Serial.println("Current calibration values:");
  for (int i=0; i < scales.get_count(); ++i) {
    // Serial.println("Before");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(calibrationValues[i], 4);
    Serial.print("; ");
    // Serial.print( (i!=scales.get_count()-1)?"\t":"\n");
  }
  Serial.println();
}

void tare() {
  Serial.println("***");
  Serial.println("Start taring");
  currentCalibrationValues();
  sendCalibratedData();
  Serial.println("- Place the load cells an a level stable surface. Remove any load applied to the load cells. After that send 't' to set the tare offset.");
  // Serial.println("Send 's' to skip setting tare offset.");

  bool _resume = false;
  char command;
  while(_resume == false){
    while(!Serial.available());
    command = get_command();
    if (!to_continue()){
      Serial.println("- Returned back. Send 't' to continue.");
      continue;
    }
    switch (command) {
        case 't':
          sendCalibratedData();
          _resume = true;
          break;
        // case 's':
        //   Serial.println("Skipping setting tare offset.");
        //   return;
        default:
          // Serial.println("Invalid input. Send 't' to tare or 's' to skip.");
          Serial.println("Invalid input. Send 't' to set the tare offset.");
    }
  }
  Serial.println("Taring...");
  bool tareSuccessful = false;
  // Tare all load cells
  unsigned long tareStartTime = millis();
  while (!tareSuccessful && millis() < (tareStartTime + TARE_TIMEOUT_SECONDS*1000)) {
    // Serial.println("Before");
    // if one of the cells fluctuated more than the allowed tolerance(still ringing), reject tare attempt;
    tareSuccessful = scales.tare(20, 10000);
    // Serial.println("After");
  }
  if (tareSuccessful){
    Serial.println("Tare successfully complete.");
    sendCalibratedData();
    Serial.println("***");
  } else {
    Serial.println("Tare failed. Timeout, check wiring and pin designations.");
    Serial.println("***");
  }
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
  Serial.println("Starting calibration...");

  tare();

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Serial.println();
    Serial.print("Calibrating ");
    Serial.print(i + 1);
    Serial.println(" load cell.");
    
    Serial.print("- Send 's' to skip calibrating of this load cell and use previous calibrating value(");
    Serial.print(calibrationValues[i], 4);
    Serial.println("). To continue calibrating, send any");

    char command;
    while (!Serial.available());
    command = get_command();
    if (command == 's') {
      continue;
    }

    // Wait for known weight input
    float calibrationValue = 1;
    float knownMass = 0;
    bool _resume = false;
    while (_resume == false) {
      Serial.println();
      Serial.print("- Place known weight(object) on load cell ");
      Serial.print(i + 1);
      Serial.println(" and enter this weight (if your object is 1kg and you want to get data in grams then enter weight in grams, e.g., 1000, if you need kg - send 1, etc.)(known weight must be > 0): ");
      // scales.read(results);
      while(!Serial.available());
      
      knownMass = Serial.parseFloat();
      if (knownMass != 0){
        Serial.print("Known weight: ");
        Serial.println(knownMass, 4);
        // Measure raw data with known weight
        scales.read(results);
        long loadValue = results[i];
        Serial.print("Not calibrated weight on this load cell: ");
        Serial.println(loadValue);

        // Do not save calculated calibration value to calibrationValues array because it may be incorrect
        calibrationValue = loadValue / knownMass;  // Calculate calibration value
        Serial.print("Calibration value for this load cell: ");
        Serial.println(calibrationValue, 4);
        Serial.print("Calibrated weight of this load cell(if it correct calculated then must be equal to known mass): ");
        Serial.println(loadValue / calibrationValue, 4);

        Serial.println("- Continue(y/n):");
        while (!Serial.available());
        command = get_command();
        if (command == 'y'){
          _resume = true;
        }
      } else {
        Serial.print("Failed to set known weight. Returned back. Known weight must be > 0");
      }
    }

    // // Measure raw data with known weight
    // scales.read(results);
    // long loadValue = results[i];
    // Serial.print("Not calibrated weight on this load cell: ");
    // Serial.println(loadValue);

    // // Do not save calculated calibration value to calibrationValues array because it may be incorrect
    // float calibrationValue = loadValue / knownMass;  // Calculate calibration value
    // Serial.print("Calibration value for this load cell: ");
    // Serial.println(calibrationValue, 4);
    // Serial.print("Calibrated weight of this load cell(if it correct calculated then must be equal to known mass): ");
    // Serial.println(loadValue / calibrationValue, 4);
    // calibrationValues cant be 0
    // if (isnan(calibrationValues[i]) || calibrationValues[i] == 0){
    //   calibrationValues[i] = 1;
    // }
    // Serial.print("Recalibrate this load cell(y/any):");
    // while (!Serial.available());
    // command = get_command();
    // if (command == 'y') {
    //   i--;
    //   continue;
    // }

    calibrationValues[i] = calibrationValue;
    
    int address = calibrationEEPROMAddress + (i * sizeof(float));

    _resume = false;
    while (_resume == false) {
      Serial.print("- Save ");
      Serial.print(calibrationValues[i]);
      Serial.print(" value to EEPROM adress ");
      Serial.print(address);
      Serial.println("?(y/n)");
      while (!Serial.available());
      command = get_command();
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
        Serial.print(calibrationValues[i], 4);
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
  currentCalibrationValues();
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