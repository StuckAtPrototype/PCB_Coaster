#include "adc_readings.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "ADC_READINGS";

#define ADC_UNIT ADC_UNIT_1
#define ADC_ATTEN ADC_ATTEN_DB_2_5
#define ADC_BITWIDTH ADC_BITWIDTH_DEFAULT

static adc_oneshot_unit_handle_t adc1_handle;

esp_err_t adc_readings_init(void) {

    // configure ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
            .unit_id = ADC_UNIT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // configuration will stay the same for all channels
    adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN,
            .bitwidth = ADC_BITWIDTH,
    };

    // configure all channels
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));
//    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config));

    return ESP_OK;
}

esp_err_t adc_readings_get_voltages(float *voltages) {
    int raw;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw);
//    voltages[0] = (float)raw / (float)(1 << ADC_BITWIDTH) * 3.3f; // Assuming 0-3.3V range
    voltages[0] = (float)raw;
//    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay


    adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &raw);
//    voltages[1] = (float)raw / (float)(1 << ADC_BITWIDTH) * 3.3f; // Assuming 0-3.3V range
    voltages[1] = (float)raw;
//    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay


    adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &raw);
//    voltages[2] = (float)raw / (float)(1 << ADC_BITWIDTH) * 3.3f; // Assuming 0-3.3V range
    voltages[2] = (float)raw;
//    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay


    adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &raw);
//    voltages[3] = (float)raw / (float)(1 << ADC_BITWIDTH) * 3.3f; // Assuming 0-3.3V range
    voltages[3] = (float)raw;
//    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay


//    adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &raw);
//    voltages[4] = (float)raw / (float)(1 << ADC_BITWIDTH) * 3.3f; // Assuming 0-3.3V range
//    voltages[4] = (float)raw;

    return ESP_OK;
}
