# Current project url -- https://wokwi.com/projects/412452836213719041

## TODO
- [ ] Check if user can send nan from serial monitor while calibrating
- 

## MAYBE TODO
// Если происходит калибровка - то происходит калибровка
// If user send inapropriate data: попросить ввести еще раз или использовать дефолтное значение
// Когда были введено число, спросить сохранить его в память или нет
    // Если сохранить, то перепроветь ячейку памяти, если там не нан, то сказать что в этой ячейке памяти есть данные и спросить перезаписать их. Написать, если нет, то новое число не будет сохранено
    // Если перезаписать, то перезаписать
    // Если нет, то написать что данные не созхранены
// Если использовать данные из памяти
// Провеить каждую ячейку на наличие нан(не число)
    // Если нан, то сказать что для такой-то лоадселл значение нан и спросить запустить ли калибровку этого датчика или использовать дефолтное значение
    // Если запустить калибровку, то запустить калибровку(см. иф выше)
    // Если нет, то этому датчику присвоить дефолтный скейл
    // Если не нан, то этому датчику присвоить дефолтный скейл
// Если использовать дефолтные значения, то всем датчикам присвоить дефолтный скейл

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