# Current project url -- https://wokwi.com/projects/412452836213719041

## TODO

- [x] Check "c" command
    - [x] Check save to memory
    - [x] Check not save to memory
- [x] Check "m" command
- [x] Check "d" command

- [x] Check "t" command
- [x] Check "c" command
- [x] Check if user can send nan from serial monitor while calibrating. User cant send nan

## BUGS

- [x] If 0 passed to calibration then nan is saved. FIXED: do not allow enter 0 as known mass
- [ ] CRITICAL(I cant repeat it more then 3 times while tests): Simulation stops while changing weight on load cells
- [ ] CRITICAL(I cant repeat it more then 1 times while tests): Simulation stop while re-taring

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