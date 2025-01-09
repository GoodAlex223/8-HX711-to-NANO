#include "HX711-multi.h"
#include <EEPROM.h>
// Include math library for isnan() to recalibrate load cells with no calibration values in EEPROM
// And if user send not a number from serial monitor
#include <math.h>

// Pins
#define CLK 2 // Common clock pin for all HX711 units
#define DOUT1 3
#define DOUT2 4
// #define DOUT3 5
// #define DOUT4 6
// #define DOUT5 7
// #define DOUT6 8
// #define DOUT7 9
// #define DOUT8 10

#define TARE_TIMEOUT_SECONDS 4

// Array of data pins
// byte DOUTS[8] = {DOUT1, DOUT2, DOUT3, DOUT4, DOUT5, DOUT6, DOUT7, DOUT8};
byte DOUTS[2] = {DOUT1, DOUT2};

// Number of load cells
#define CHANNEL_COUNT sizeof(DOUTS) / sizeof(byte)

// Calibration values for each load cell
// float CALIBRATION_VALUES[CHANNEL_COUNT] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
float CALIBRATION_VALUES[CHANNEL_COUNT] = {1.0, 1.0};
// float CALIBRATION_VALUES[CHANNEL_COUNT] = {0.6667, 17492.8339};
// float CALIBRATION_VALUES[CHANNEL_COUNT] = {0.42, 0.42, 0.42, 0.42, 0.42, 0.42, 0.42, 0.42};
const int CALIBRATION_EEPROM_ADDRESS = 0;  // Starting EEPROM address for calibration values


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
HX711MULTI SCALES(CHANNEL_COUNT, DOUTS, CLK); // default value 128. Then GAIN = 1
// HX711MULTI SCALES(CHANNEL_COUNT, DOUTS, CLK, 128); // GAIN = 1
// HX711MULTI SCALES(CHANNEL_COUNT, DOUTS, CLK, 64); // GAIN = 3
// HX711MULTI SCALES(CHANNEL_COUNT, DOUTS, CLK, 32); // GAIN = 2
long RESULTS[CHANNEL_COUNT];
uint32_t NUMBER_OF_READ = 0;

// #define BUFFER_SIZE 64

void setup() {
  // Higher volumes of data or frequent calls to Serial.print can cause delays in the program, 
  // especially at low baud rates.
  // Possible baud rates:
  // 300: For extremely simple or low-power applications (e.g., weather stations).
  // 4800: For very long cables or noisy environments.
  // 9600: Default and reliable for basic Arduino projects. Debugging or basic monitoring
  // 38400 or 57600: Medium-speed communication
  // 115200: Real-time data logging
  // 250000 or higher: High-speed data transfer
  Serial.begin(57600);
  // https://docs.arduino.cc/language-reference/en/variables/utilities/PROGMEM/
  Serial.println(F("Starting setuping..."));
  // // For some reason, if you add this pin, the console starts to display the text
  // pinMode(11, OUTPUT);
  // Serial.print doesn't guarantee immediate transmission; it writes to the buffer, which is sent in the background.
  // Use Serial.flush() to wait for the buffer to clear
  // Serial.flush();
  // delay(100);
  // currentCalibrationValues();
  // tare();
  // Serial.println(()pow(10.0, 10));

  bool _resume = false;
  // char command[BUFFER_SIZE];
  char command;
  while (_resume == false) {
    // Ask if you want to calibrate, use values from memory, or use default values
    Serial.println(F("Commands:"));
    Serial.println(F("- Send 'c' to change calibration values of load cells(with taring)"));
    Serial.println(F("- Send 'm' to load calibration values from EEPROM(with taring)"));
    Serial.println(F("- Send 'd' to use default calibration values(without taring)"));
    
    // while (!Serial.available());

    // get_command(command);
    command = get_command(true);

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
      // // TEST
      // float _calibrationValues[CHANNEL_COUNT] = {420.0, 419.8443};
      // for (int i=0; i < SCALES.get_count(); ++i) {
      //   saveCalibrationToEEPROM(i, _calibrationValues);
      // }
      // // float* loadedValues = loadCalibrationFromEEPROM();
      // // memcpy(CALIBRATION_VALUES, loadedValues, sizeof(float) * CHANNEL_COUNT);
      tare();
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
  sendLoopCommandHints();
  Serial.println(F("***"));
  // Serial.flush();
}

void loop() {
  // Serial.println(F("ABOBA"));
  if (Serial.available() > 0) {
    // char command[BUFFER_SIZE];
    char command;
    // get_command(command);
    command = get_command(false);
    // command = get_command(true);
    // Serial.println(F("ABOBA 2"));
    if ((is_command_special(command) == false) && (to_continue(true) == true)){
      // if (strcmp(command, "t") == 0) tare();
      // else if (strcmp(command,  "c") == 0) calibrate();
      if (command == 't') tare();
      else if (command == 'c') calibrate();
      else if (command == 'm') loadCalibrationFromEEPROM();
      else {
        Serial.println(F("UNKNOWN command"));
        sendLoopCommandHints();
      }
      // else if (command == 'e') changeSavedCalFactor(); //edit calibration value manually
    }
  }
  // Do not use F() because of big number of calls
  Serial.print("Number of read: ");
  Serial.print(NUMBER_OF_READ++);
  Serial.print("; ");
  // NUMBER_OF_READ++;

  sendCalibratedData();

  delay(1000);
}

void sendLoopCommandHints(){
  Serial.println(F("Available commands:"));
  Serial.println(F("- Send 'c' to change calibration values of load cells."));
  Serial.println(F("- Send 't' to set tare offset for all load cells."));
  Serial.println(F("- Send 'm' to load calibration values from EEPROM."));
  Serial.println(F("- Send any key to show this hint."));
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

void is_scales_not_ready_warning(){
  // wait for all the chips to become ready
  // from the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
  // input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
  while (!SCALES.is_ready()){
    // HX711MULTI.read or HX711MULTI.readRaw do not send warning that HX711 is not ready
    Serial.println(F("WARNING: Some of load cells are busy or disconected."));
    delay(1000);
  }
}

bool is_command_special(char command){
  bool result;
  result = (command == '\n' || command == ' ' || command == '\r');
  // Serial.println();
  return result;
}

char get_command(bool block_specials){
  Serial.println();
  while (!Serial.available());
  // char command = Serial.read();
  char command;
  // while (command == '\n'){
  // Serial.println(F("ABOBA 1.5"));
  while (Serial.available() > 0){
    // Serial.println(F("ABOBA 1.7"));
    command = Serial.read();
    // Serial.println(command);
    // Serial.println(F("ABOBA 1.8"));
    // Wait for correct char
    if ((block_specials == true) && (is_command_special(command) == true)){
      while(!Serial.available());
    } 
    // Serial.println();
    // Serial.flush();
    else {
      // Skip other characters because correct command was get
      while(Serial.available() > 0){
        Serial.read();
        // Serial.print(""); // I do not why but wth this line all wotk correctly
        
      };
    }
  }
  // Send info message about user input if command is not special symbol
  if (is_command_special(command) == false){
    Serial.print(F("Your input: '"));
    Serial.print(command);
    Serial.println(F("'."));
  }
  return command;
}

bool to_continue(bool question) {
  if (question){
    Serial.println(F("- Continue?('y' to continue or 'n' to return back):"));
  }
  // char command[BUFFER_SIZE];
  char command;
  while (true){
    // while (!Serial.available());
    command = get_command(true);
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
  // Serial.println("Before");
  is_scales_not_ready_warning();
  // Do not use F() because of big number of calls(in loop function)
  Serial.print("Current calibrated weights ");
  SCALES.read(RESULTS);
  // Serial.print("After");
  // for (int i=0; i < SCALES.get_count(); ++i) {
  for (int i=0; i < CHANNEL_COUNT; ++i) {
    // Serial.println("Before");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(RESULTS[i] / CALIBRATION_VALUES[i], 2);
    Serial.print("; ");
    // Serial.println("After");
    // Serial.print( (i!=SCALES.get_count()-1)?"\t":"\n");
  }
  // Serial.println("After 2");
  Serial.println();
}

void currentCalibrationValues() {
  Serial.println(F("Current calibration values"));
  for (int i=0; i < CHANNEL_COUNT; ++i) {
    // Serial.println("Before");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(CALIBRATION_VALUES[i], 4);
    Serial.print("; ");
    // Serial.print( (i!=SCALES.get_count()-1)?"\t":"\n");
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
    // while(!Serial.available());
    // get_command(command);
    command = get_command(true);

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
    is_scales_not_ready_warning();
    // if one of the cells fluctuated more than the ALLOWED TOLERANCE(still ringing), reject tare attempt;
    // tareSuccessful = SCALES.tare(20, 10000);
    tareSuccessful = SCALES.tare(20, 420);
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
  //   if (isnan(CALIBRATION_VALUES[i])){
  //     Serial.println("ABOBA");
  //   }
  // }
  Serial.println(F("***"));
  Serial.println(F("Starting calibration..."));

  tare();
  int load_cell_number;

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    load_cell_number = i + 1;
    Serial.println();
    Serial.print(F("Calibrating "));
    Serial.print(load_cell_number);
    Serial.println(F(" load cell."));
    
    // Serial.print(F("- To skip calibrating of this load cell and use previous calibrating value("));
    // Serial.print(CALIBRATION_VALUES[i], 4);
    // Serial.println(F(") send 'y', to continue -- 'n'"));

    // if (to_continue(false)){
    //   continue;
    // }

    // Wait for known weight input
    float knownWeight = 0;
    float calibrationValue = 1;
    float calibratedWeight;
    bool _resume = false;
    while (_resume == false) {
      Serial.println();
      Serial.print(F("- Place known weight(object) on load cell "));
      Serial.print(load_cell_number);
      Serial.println(F(" and enter this weight (if your object is 1kg and you want to get data in grams then enter weight in grams, e.g., 1000, if you need kg - send 1, etc.)(known weight must be > 0): "));
      // SCALES.read(RESULTS);

      while (true) {
        while(!Serial.available());
        knownWeight = Serial.parseFloat();
        if (knownWeight == 0){
          Serial.println(F("- Known weight must be > 0. Send correct known weight"));
        } else {
          break;
        }
      }
      // if (knownWeight != 0){

        Serial.println();
        Serial.print(F("Your knownWeight = "));
        Serial.println(knownWeight, 4);
        Serial.println(F("Calculation of calibrationValue(if calibrationValue correctly calculated then calibratedWeight = knownWeight):"));
        // Measure raw data with known weight
        is_scales_not_ready_warning();
        SCALES.read(RESULTS);
        long notCalibratedWeight = RESULTS[i];
        Serial.print(F("notCalibratedWeight(from load cell) = "));
        Serial.println(notCalibratedWeight);
        // Do not save calculated calibration value to CALIBRATION_VALUES array because it may be incorrect
        calibrationValue = notCalibratedWeight / knownWeight;  // Calculate calibration value
        Serial.print(F("calibrationValue = notCalibratedWeight / knownWeight = "));
        Serial.println(calibrationValue, 4);
        Serial.print(F("calibratedWeight = notCalibratedWeight / calibrationValue = "));
        calibratedWeight = notCalibratedWeight / calibrationValue;
        Serial.println(calibratedWeight, 4);

        if (isnan(calibratedWeight)){
          Serial.print(F("FAILED: calibratedWeight is not a number(nan). Recheck that load cell "));
          Serial.print(load_cell_number);
          Serial.println(F(" is connected, object is placed on load cell and you input its weight"));
        } else if (to_continue(true)){
          _resume = true;
        }
      // } else {
      //   Serial.print(F("- Invalid known weight. Returned back. Known weight must be > 0. Send correct known weight"));
      // }
    }

    // // Measure raw data with known weight
    // SCALES.read(RESULTS);
    // long notCalibratedWeight = RESULTS[i];
    // Serial.print("Not calibrated weight on this load cell: ");
    // Serial.println(notCalibratedWeight);

    // // Do not save calculated calibration value to CALIBRATION_VALUES array because it may be incorrect
    // float calibrationValue = notCalibratedWeight / knownWeight;  // Calculate calibration value
    // Serial.print("Calibration value for this load cell: ");
    // Serial.println(calibrationValue, 4);
    // Serial.print("Calibrated weight of this load cell(if it correct calculated then must be equal to known mass): ");
    // Serial.println(notCalibratedWeight / calibrationValue, 4);
    // CALIBRATION_VALUES cant be 0
    // TODO: Add accepting or rejecting of calculated calibration value
    // if (isnan(CALIBRATION_VALUES[i]) || CALIBRATION_VALUES[i] == 0){
    //   CALIBRATION_VALUES[i] = 1;
    // }
    // Serial.print("Recalibrate this load cell(y/any):");
    // while (!Serial.available());
    // command = get_command();
    // if (command == 'y') {
    //   i--;
    //   continue;
    // }

    CALIBRATION_VALUES[i] = calibrationValue;

    saveCalibrationToEEPROM(i, CALIBRATION_VALUES);
  }

  Serial.println(F("End calibration"));
  currentCalibrationValues();
  Serial.println(F("***"));
  delay(500);
}

void saveCalibrationToEEPROM(int i, float _calibrationValues[]) { 
    int address = CALIBRATION_EEPROM_ADDRESS + (i * sizeof(float));

    bool _resume = false;
    while (_resume == false) {
      Serial.print(F("- Save "));
      Serial.print(_calibrationValues[i]);
      Serial.print(F(" value to EEPROM adress "));
      Serial.print(address);
      Serial.println(F("?(y/n)"));
      if (to_continue(false)){
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.begin(512);
// #endif
        // Save calibration value to EEPROM
        EEPROM.put(address, _calibrationValues[i]);
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.commit();
// #endif
        EEPROM.get(address, _calibrationValues[i]);
        Serial.print(F("Value "));
        Serial.print(_calibrationValues[i], 4);
        Serial.print(F(" saved to EEPROM address: "));
        Serial.println(address);
        _resume = true;
      } else {
        Serial.println(F("Value not saved to EEPROM"));
        _resume = true;
      }
      Serial.println();
    }
}

// float* loadCalibrationFromEEPROM() {
void loadCalibrationFromEEPROM() {
  // static float _calibrationValues[CHANNEL_COUNT];
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    int address = CALIBRATION_EEPROM_ADDRESS + (i * sizeof(float));
    // EEPROM.get(address, _calibrationValues[i]);
    EEPROM.get(address, CALIBRATION_VALUES[i]);
    Serial.print(F("Loaded calibration value for load cell "));
    Serial.print(i + 1);
    Serial.print(F(": "));
    // Serial.println(_calibrationValues[i]);
    Serial.println(CALIBRATION_VALUES[i]);
    // if (isnan(CALIBRATION_VALUES[i])){
    //   Serial.println("Calibrate value is nan. Reasigning");
    //   CALIBRATION_VALUES[i] = 1;
    //   Serial.println("New calibration value for load cell ");
    //   Serial.print(i + 1);
    //   Serial.print(": ");
    //   Serial.println(CALIBRATION_VALUES[i]);
    // }
  };
  // return _calibrationValues;
}