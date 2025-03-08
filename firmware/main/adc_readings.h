#ifndef BATTERY_H
#define BATTERY_H

#include "esp_err.h"

// Initializes the ADC for all channels
esp_err_t adc_readings_init(void);

// Reads the voltages
// voltages is an array[5]
esp_err_t adc_readings_get_voltages(float *voltages);

#endif // BATTERY_H
