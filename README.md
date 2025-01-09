# Current project url -- https://wokwi.com/projects/412452836213719041

## TODO

## BUGS

- [x] If 0 passed to calibration then nan is saved. FIXED: do not allow enter 0 as known mass
- [ ] CRITICAL(I cant repeat it more then 3 times while tests): Simulation stops while changing weight on load cells
- [ ] CRITICAL(I cant repeat it more then 1 times while tests): Simulation stop while re-taring
- [ ] CRITICAL(I cant repeat it more then 1 times while tests): Simulation stop after calibration

## MAYBE TODO

// If calibration occurs - calibration occurs
// If user send inapropriate data: ask to enter again or use default value
// When a number has been entered, ask to save it to memory or not
    // If save, then re-compute the memory location, if there is no nan there, then say that there is data in this memory location and ask to overwrite it. Write, if not, the new number will not be saved
    // If overwrite, then overwrite
    // If not, write that the data is not stored
// If using data from memory
// Check each cell for nan(not a number)
    // If nan, then say that for such and such lowadsell the value is nan and ask whether to start calibration of this sensor or use the default value.
    // If run calibration, then run calibration(see if above)
    // If not, assign a default skale to this sensor
    // If not nan, assign a default skail to this sensor
// If default values are used, assign a default skail to all sensors

- [ ] Ideas from https://wokwi.com/projects/407626388355231745

## Project sources

- Demo with HX711 Library -- https://wokwi.com/projects/344192176616374868
- hx711_ADC_Calibration.ino -- https://wokwi.com/projects/407626388355231745
- HX711-multi github -- https://github.com/compugician/HX711-multi/tree/master
- https://dfimg.dfrobot.com/enshop/image/data/SEN0160/hx711_english.pdf
- https://docs.wokwi.com/parts/wokwi-hx711/


## Wokwi work

1. Set all weights on load cells to 0
2. Restart simulation
3. Start calibration
4. Set weight to 1kg on first load cell
5. Enter 1000 to console
6. Repeat 4-5
7. Check correctness of different load cells with different weights

## DONE

### 25_01_08

- Uppercased some global constants(SCALES, RESULTS, NUMBER_OF_READ, CALIBRATION_VALUES, CALIBRATION_EEPROM_ADDRESS)
- `setup`: Added descriptions to command hints for user
- `loop`: Added ability to load calibration values from EEPROM by command from user
- Created `sendLoopCommandsHints` function to avoid repeating of command hints. Added to `setup` and `loop`
- Created `is_scales_not_ready_warning` that use HX711MULTI.is_ready to send warning message and block code execution if HX711 load cells are busy or disconnected. Default behavior of HX711MULTI library is block of code without any message
- Added `is_scales_not_ready_warning` before every use of HX711MULTI class to avoid code blocking without message
    - `sendCalibratedData`: Added `is_scales_not_ready_warning` at start
    - `tare`: Added `is_scales_not_ready_warning` before taring
    - `calibrate`: Added `is_scales_not_ready_warning` before HX711MULTI.read
- `get_command`: Moved empty line print to start of function to show that user input was get even if not processed
- Replaced HX711MULTI.get_count call with already created constant CHANNEL_COUNT because under the hood HX711MULTI.get_count returns CHANNEL_COUNT from instance initialization
    - `sendCalibratedData`
    - `currentCalibrationValues`
- `tare`:
    - Decreased allowed tolerance from 10000(~24kg) to 420(1kg) for load cells taring
- `calibrate`:
    - Created `load_cell_number` variable that is printed to user as load cell index + 1. To replace repeated code
    - Added number of load cell to fail message if `calibratedWeight` is nan
- Created `saveCalibrationToEEPROM`. Used in tests and in `calibrate` function

### 24_11_30

- The command from the Arduino IDE console is sent with a newline character. In order to avoid the following command requests that are in the code, a character filter was added earlier, which prohibited sending some characters ('\n', '\r', ' ' '). But for some reason it didn't fully work: instead of a single character, a word or command could be sent with several special characters (e.g. “s\n ‘ or ’ s”), which were then used as the next command. Such characters are ignored and the code continues to wait for the correct command, but this is not correct in the case of receiving a command in a loop: when a character appears in the console, the loop is interrupted to receive the command, and if the command consists of special characters, the code is blocked until the correct command or any character is received. This was the case, but now special characters in the loop do not block execution, but are simply ignored

### 24_11_25

- [x] Check "c" command
    - [x] Check save to memory
    - [x] Check not save to memory
- [x] Check "m" command
- [x] Check "d" command
- [x] Check "t" command
- [x] Check "c" command
- [x] Check if user can send nan from serial monitor while calibrating. User cant send nan

- Added descriptive comments
- Changed type of number_of_read from int to uint32_t
- Added static strings (F()) to store constant data in flash memory and free up RAM
- Removed taring at the start of program execution
- Added descriptive, warning and error messages to user
- Added commented function as option to get commands from user
- Changed get_command function: added input check to not get '\n', ' ' and '\r' chars as command
- Changed "bool to_continue(bool question)" function: bool question is used to send or no "Continue?" message to user
- Changed sendCalibratedData: Decreased number of digits after dot in output to monitor of calibrated weight, to possibly decrease chance of program block(critical bug)
- Changed calibrate function:
    - Commented skip of load cell calibration to reduce steps of calibration for user
    - Added check of inputed knownWeight so it is always not equal to 0
    - Added check of calibrated weight with isnan to assure that calibration value is correctly calculated

### 24_11_06

- Fixed incorrect text after successful calibration.
- Added confirmation of module calibration value change during calibration.
- Added more descriptive text to make the setup and output processes easier to understand.
- Added ability to view and confirm your input in the console.

### 24_10_29

- Adapted everything from source ( https://github.com/olkal/HX711_ADC/blob/master/examples/Calibration/Calibration.ino )
- except changeSavedCalFactor(manual change of calibration value) and output data only when it changes(smooth data output; will create additional load)
- Also added some text commands, for convenience
- In simulation it is not possible to test the operation of the memory module, so whether it works or not, I don't know