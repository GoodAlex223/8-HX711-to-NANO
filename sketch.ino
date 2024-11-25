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
// byte DOUTS[2] = {DOUT1, DOUT2};

// Calibration values for each load cell
float calibrationValues[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
// float calibrationValues[2] = {1.0, 1.0};
// float calibrationValues[2] = {0.6667, 17492.8339};
// float calibrationValues[8] = {0.42, 0.42, 0.42, 0.42, 0.42, 0.42, 0.42, 0.42};
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
//  128: channel A, gain value 128
//  64: channel A, gain value 64
//  32: channel B, gain value 32
// initialization types: HX711MULTI(int count, byte *dout, byte pd_sck, byte gain = 128);
HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK); // default value 128. Then GAIN = 1
// HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK, 128); // GAIN = 1
// HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK, 64); // GAIN = 3
// HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK, 32); // GAIN = 2
long results[CHANNEL_COUNT];
uint32_t number_of_read = 0;

// #define BUFFER_SIZE 64

void setup() {
  // Possible baud rates:
  // Low
  // 9600, 57600 etc.
  // High
  // 115200, 250000
  Serial.begin(57600);
  // https://docs.arduino.cc/language-reference/en/variables/utilities/PROGMEM/
  Serial.println(F("Starting setuping..."));
  delay(100);
  Serial.flush();
  // For some reason, if you add this pin, the console starts to display the text
  pinMode(11, OUTPUT);
  // currentCalibrationValues();
  // tare();
  // Serial.println(()pow(10.0, 10));

  bool _resume = false;
  // char command[BUFFER_SIZE];
  char command;
  while (_resume == false) {
    // Ask if you want to calibrate, use values from memory, or use default values
    Serial.println(F("Commands:"));
    Serial.println(F("- Send 'c' to change calibration values of load cells"));
    Serial.println(F("- Send 'm' to load calibration values from EEPROM"));
    Serial.println(F("- Send 'd' to use default calibration values"));
    
    while (!Serial.available());

    // get_command(command);
    command = get_command();

    if (!to_continue(true)){
      Serial.println(F("- Returned back. Use one of above commands"));
      continue;
    }

    // if (strcmp(command, "c") == 0){
    if (command == 'c'){
      calibrate();
      _resume = true;
    // } else if (strcmp(command, "m") == 0){
    } else if (command == 'm'){
      loadCalibrationFromEEPROM();
      _resume = true;
    // } else if (strcmp(command, "d") == 0){
    } else if (command == 'd'){
      Serial.println(F("Using default calibration values."));
      currentCalibrationValues();
      _resume = true;
    } else {
      Serial.println(F("- INVALID command. Try again."));
    }
  }
  Serial.println(F("Setup complete. Ready for commands."));
  Serial.println(F("***"));
  Serial.println(F("- Send 'c' to change calibration values of load cells."));
  Serial.println(F("- Send 't' to set tare offset for all load cells."));
  // Serial.print doesn't guarantee immediate transmission; it writes to the buffer, which is sent in the background.
  // Use Serial.flush() to wait for the buffer to clear
  Serial.flush();
}

void loop() {
  if (Serial.available() > 0) {
    Serial.println(F("Commands:"));
    Serial.println(F("- Send 'c' to change calibration values of load cells."));
    Serial.println(F("- Send 't' to set tare offset for all load cells."));
    // char command[BUFFER_SIZE];
    char command;
    // get_command(command);
    command = get_command();
    if (to_continue(true)){
      // if (strcmp(command, "t") == 0) tare();
      // else if (strcmp(command,  "c") == 0) calibrate();
      if (command == 't') tare();
      else if (command == 'c') calibrate();
      // else if (command == 'e') changeSavedCalFactor(); //edit calibration value manually
    }
  }
  // Do not use F() because of big number of calls
  Serial.print("Number of read: ");
  Serial.println(number_of_read++);
  // number_of_read++;

  sendCalibratedData();

  delay(1000);
}


// void get_command(char* destBuffer) {
//   char receivedChar;
//   static char inputBuffer[BUFFER_SIZE];
//   byte bufferIndex = 0;           // Current position in the buffer
  
//   while (Serial.available() > 0 && bufferIndex == 0) {
//     receivedChar = Serial.read();  // Read the incoming character
    
//     // If newline is detected, process the input
//     if (receivedChar == '\n' || receivedChar == '\r') {
//       if (bufferIndex == 0){
//         // Serial.println(F("- Your input: '\\n'. Send correct command, please"));
//         // Do not allow send new line as first char
//         while(!Serial.available());
//         // inputBuffer[bufferIndex] = '\0';  // Null-terminate the string
//       // }
//       } else {
//         break;
//       }
//       // if (bufferIndex > 0) {  // Ensure the buffer is not empty
//       //   inputBuffer[bufferIndex] = '\0';  // Null-terminate the string
//       //   Serial.print(F("Your input: '"));
//       //   Serial.print(inputBuffer);       // Process the complete input
//       //   Serial.println(F("'."));
//       //   bufferIndex = 0;                 // Reset the buffer
//       //   return inputBuffer;
//       // }
//       // break;
//       // continue;
//     } 
//     else if (bufferIndex < BUFFER_SIZE - 1) {
//     // else if (bufferIndex < buffer_size - 1) {
//     // if (bufferIndex < BUFFER_SIZE - 1) {
//       // Add character to buffer if there's space
//       inputBuffer[bufferIndex++] = receivedChar;
//     } else {
//       // If the buffer is full, discard input (or handle overflow)
//       Serial.println(F("Input too long, discarding."));
//       bufferIndex = 0;  // Reset the buffer
//     }
//   }
//   // Always null-terminate the string and write it to destBuffer
//   // to avoid saving previous destBuffer in case of new line or input discarding
//   inputBuffer[bufferIndex] = '\0';  // Null-terminate the string
//   strncpy(destBuffer, inputBuffer, BUFFER_SIZE);
//   if (bufferIndex > 0) {  // Ensure the buffer is not empty
//     Serial.print(F("Your input: '"));
//     Serial.print(destBuffer);       // Process the complete input
//     Serial.println(F("'."));
//     // bufferIndex = 0;                 // Reset the buffer
//     // return inputBuffer;
//   }
// }


char get_command(){
  // char command = Serial.read();
  char command;
  // while (command == '\n'){
  while (Serial.available() > 0){
    command = Serial.read();
    // Wait for correct char
    if (command == '\n' || command == ' ' || command == '\r'){
      while(!Serial.available());
    }
  }
  Serial.print(F("Your input: '"));
  Serial.print(command);
  Serial.println(F("'."));
  return command;
}

bool to_continue(bool question) {
  if (question){
    Serial.println(F("- Continue?('y' to continue or 'n' to return back):"));
  }
  // char command[BUFFER_SIZE];
  char command;
  while (true){
    while (!Serial.available());
    command = get_command();
    // get_command(command);

    // if (strcmp(command, "y") == 0){
    if (command == 'y'){
      return true;
    // } else if (strcmp(command, "n") == 0){
    } else if (command == 'n'){
      return false;
    } else {
      Serial.println(F("- INVALID input. Send 'y' or 'n'"));
    }
  }
}

void sendCalibratedData() {
  // Do not use F() because of big number of calls(in loop function)
  Serial.println("Current calibrated weights:");
  // Serial.println("Before");
  scales.read(results);
  // Serial.println("After");
  for (int i=0; i < scales.get_count(); ++i) {
    // Serial.println("Before");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(results[i] / calibrationValues[i], 2);
    Serial.print("; ");
    // Serial.println("After");
    // Serial.print( (i!=scales.get_count()-1)?"\t":"\n");
  }
  // Serial.println("After 2");
  Serial.println();
}

void currentCalibrationValues() {
  Serial.println(F("Current calibration values:"));
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
  Serial.println(F("***"));
  Serial.println(F("Start taring"));
  currentCalibrationValues();
  sendCalibratedData();
  Serial.println(F("- Place the load cells an a level stable surface. Remove any load applied to the load cells. After that send 't' to set the tare offset."));
  // Serial.println("Send 's' to skip setting tare offset.");

  bool _resume = false;
  // char command[BUFFER_SIZE];
  char command;
  while(_resume == false){
    while(!Serial.available());
    // get_command(command);
    command = get_command();

    if (!to_continue(true)){
      Serial.println(F("- Returned back. Send 't' to continue."));
      continue;
    }
    // switch (command) {
    //     case 't':
    //       sendCalibratedData();
    //       _resume = true;
    //       break;
    //     // case 's':
    //     //   Serial.println("Skipping setting tare offset.");
    //     //   return;
    //     default:
    //       // Serial.println("Invalid input. Send 't' to tare or 's' to skip.");
    //       Serial.println(F("Invalid input. Send 't' to set the tare offset."));
    // }
    // if (strcmp(command, "t") == 0){
    if (command == 't'){
      sendCalibratedData();
      _resume = true;
    // case 's':
    //   Serial.println("Skipping setting tare offset.");
    //   return;
    } else {
      // Serial.println("Invalid input. Send 't' to tare or 's' to skip.");
      Serial.println(F("- INVALID input. Send 't' to set the tare offset."));
    }
  }
  Serial.println(F("Taring..."));
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
    Serial.println(F("Tare successfully complete."));
    sendCalibratedData();
  } else {
    Serial.println(F("FAILED: Timeout, check wiring and pin designations."));
  }
  Serial.println(F("***"));
}

// Calibrate each load cell individually
void calibrate() {
  // loadCalibrationFromEEPROM();
  // for (int i = 0; i < CHANNEL_COUNT; i++) {
  //   if (isnan(calibrationValues[i])){
  //     Serial.println("ABOBA");
  //   }
  // }
  Serial.println(F("***"));
  Serial.println(F("Starting calibration..."));

  tare();

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Serial.println();
    Serial.print(F("Calibrating "));
    Serial.print(i + 1);
    Serial.println(F(" load cell."));
    
    // Serial.print(F("- To skip calibrating of this load cell and use previous calibrating value("));
    // Serial.print(calibrationValues[i], 4);
    // Serial.println(F(") send 'y', to continue -- 'n'"));

    // if (to_continue(false)){
    //   continue;
    // }

    // Wait for known weight input
    float knownWeight = 0;
    float calibrationValue = 1;
    bool _resume = false;
    while (_resume == false) {
      Serial.println();
      Serial.print(F("- Place known weight(object) on load cell "));
      Serial.print(i + 1);
      Serial.println(F(" and enter this weight (if your object is 1kg and you want to get data in grams then enter weight in grams, e.g., 1000, if you need kg - send 1, etc.)(known weight must be > 0): "));
      // scales.read(results);

      while(!Serial.available());
      knownWeight = Serial.parseFloat();
      while(knownWeight == 0){
        knownWeight = Serial.parseFloat();
      }
      // if (knownWeight != 0){
        Serial.print(F("Your knownWeight = "));
        Serial.println(knownWeight, 4);
        Serial.println(F("Calculation of calibrationValue(if calibrationValue correctly calculated then calibratedWeight = knownWeight):"));
        // Measure raw data with known weight
        scales.read(results);
        long notCalibratedWeight = results[i];
        Serial.print(F("notCalibratedWeight(from load cell) = "));
        Serial.println(notCalibratedWeight);
        // Do not save calculated calibration value to calibrationValues array because it may be incorrect
        calibrationValue = notCalibratedWeight / knownWeight;  // Calculate calibration value
        Serial.print(F("calibrationValue = notCalibratedWeight / knownWeight = "));
        Serial.println(calibrationValue, 4);
        Serial.print(F("calibratedWeight = notCalibratedWeight / calibrationValue = "));
        Serial.println(notCalibratedWeight / calibrationValue, 4);

        if (isnan(notCalibratedWeight / calibrationValue)){
          Serial.print(F("FAILED: calibratedWeight is not a number(nan). Recheck that object is placed on load cell and you input its weight"));
        } else if (to_continue(true)){
          _resume = true;
        }
      // } else {
      //   Serial.print(F("- Invalid known weight. Returned back. Known weight must be > 0. Send correct known weight"));
      // }
    }

    // // Measure raw data with known weight
    // scales.read(results);
    // long notCalibratedWeight = results[i];
    // Serial.print("Not calibrated weight on this load cell: ");
    // Serial.println(notCalibratedWeight);

    // // Do not save calculated calibration value to calibrationValues array because it may be incorrect
    // float calibrationValue = notCalibratedWeight / knownWeight;  // Calculate calibration value
    // Serial.print("Calibration value for this load cell: ");
    // Serial.println(calibrationValue, 4);
    // Serial.print("Calibrated weight of this load cell(if it correct calculated then must be equal to known mass): ");
    // Serial.println(notCalibratedWeight / calibrationValue, 4);
    // calibrationValues cant be 0
    // TODO: Add accepting or rejecting of calculated calibration value
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
      Serial.print(F("- Save "));
      Serial.print(calibrationValues[i]);
      Serial.print(F(" value to EEPROM adress "));
      Serial.print(address);
      Serial.println(F("?(y/n)"));
      if (to_continue(false)){
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.begin(512);
// #endif
        // Save calibration value to EEPROM
        EEPROM.put(address, calibrationValues[i]);
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.commit();
// #endif
        EEPROM.get(address, calibrationValues[i]);
        Serial.print(F("Value "));
        Serial.print(calibrationValues[i], 4);
        Serial.print(F(" saved to EEPROM address: "));
        Serial.println(address);
        _resume = true;
      } else {
        Serial.println(F("Value not saved to EEPROM"));
        _resume = true;
      }
    }
  }

  Serial.println(F("End calibration"));
  currentCalibrationValues();
  Serial.println(F("***"));
  delay(500);
}

void loadCalibrationFromEEPROM() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    int address = calibrationEEPROMAddress + (i * sizeof(float));
    EEPROM.get(address, calibrationValues[i]);
    Serial.print(F("Loaded calibration value for load cell "));
    Serial.print(i + 1);
    Serial.print(F(": "));
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