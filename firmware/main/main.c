#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_log.h"
#include <math.h>

#include "ws2812_control.h"
#include "adc_readings.h"
#include "driver/temperature_sensor.h"

#include "screen.h"
#include "user_interface.h"

#include "weight_lookup.h"
#include "water_state.h"
#include "led_color_lib.h"

#include "button_handler.h"

static const char *TAG = "main";

temperature_sensor_handle_t temp_sensor = NULL;

struct led_state new_state1;

uint32_t last_active_screen = 0;


float get_average_weight_counts(void){
    float voltages[5];

    // Call the function to get voltages
    adc_readings_get_voltages(voltages);

    float avg = 0;
    for(int i = 0; i < 4; i++)
        avg+=voltages[i];
    avg = avg/4;

    return avg;
}

void read_and_print_voltages(void) {

    // add a weight reading to our
    add_weight_reading(lookup_and_extrapolate(get_average_weight_counts()));

    // esp32 internal temperature measurement
    float tsens_value;
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &tsens_value));
    add_temperature_reading(tsens_value);

    // run cup logic
    process_cup();

    // screen text buffer
    char screen_text[32];

    // update screen 0 vars
    sprintf(screen_text, "sips: %i", water_state.sip_count);
    ui_set_scr_0_label_0_text(screen_text);

    sprintf(screen_text, "refill: %i", water_state.refill_count);
    ui_set_scr_0_label_1_text(screen_text);

    // update screen 1 vars
    // calculate minites remaining (rounded up)
    int minutes_remaining = REMINDER_INTERVAL_MINUTES - ceil((water_state.time_elapsed_since_last_event / 60000000.0)) + 1;
    int percentage_remaining = ceil(minutes_remaining / ((float)REMINDER_INTERVAL_MINUTES + 1) * 100.0);

    // issue a warning symbol if its been -20 minutes -- a bit of an easter egg :)
    if(minutes_remaining < -20){
        sprintf(screen_text, "%s", LV_SYMBOL_WARNING);
    } else {
        sprintf(screen_text, "%i", minutes_remaining);
    }
    ui_set_scr_1_label_0_text(screen_text);
    ui_set_scr_1_bar_0_value(percentage_remaining);

    sprintf(screen_text, "sips: %i", water_state.sip_count);
    ui_set_scr_1_label_1_text(screen_text);

    uint32_t color;
    float temp = temperature_average();
    if(water_state.drink_reminder == 1){
        // pulse light
        color = get_pulsing_color(0, 255, 0); // green
    } else {
        if(temp > 0){
            color = get_color_between_blue_red(temp);
        } else {
            color = get_next_color_full_spectrum();
        }
    }

    if(water_state.cup_state == CUP_NOT_PRESENT){
        // we need to be able to clear this screen with a button press or time
        if(water_state.time_elapsed_since_last_event < 2500000){  // 2500000 == 2.5 seconds
            if(ui_get_active_screen_num() != UI_SCREEN_SPLASH){
                ui_set_active_screen(UI_SCREEN_SPLASH);
            }
        } else {
            ui_set_active_screen(last_active_screen);
        }

    } else {
       if(ui_get_active_screen_num() == UI_SCREEN_SPLASH){
            ui_set_active_screen(last_active_screen);
        }
    }


    // write the leds
    for(int i = 0; i < 11; i++)
        new_state1.leds[i] = color;
    ws2812_write_leds(new_state1);

}

/* Drivers
 * (x) WS2812
 * (X) Display
 * (X) ADC
 */

/* Logical Drivers
 * - light up LEDS
 * - drive display
 * - measure resistance
 * -- temperature
 * -- weight
 * - calculate liquid consumption
 * - read button (interrupt)
 */
void app_main(void)
{
    ESP_LOGI(TAG, "starting up");

    // init the screen and lvgl
    screen_init();

    // initiate the active screen variable
    last_active_screen = ui_get_active_screen_num();

    // initialize ADC
    adc_readings_init();

    // init led driver
    ws2812_control_init();
    ESP_LOGI(TAG, "ws driver initialized!!");

    // temperature sensor
    ESP_LOGI(TAG, "Install temperature sensor, expected temp ranger range: 10~50 ℃");
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));

    ESP_LOGI(TAG, "Enable temperature sensor");
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));

    // enable button
    button_handler_init();

    while (1) {

        read_and_print_voltages();

        // read in the button buffer
        QueueHandle_t button_queue = button_get_queue_handle();
        button_event_t event_data;
        if(xQueueReceive(button_queue, &event_data, 0)){
            if(event_data == BUTTON_SHORT_PRESS){
                // switch the active user interface to the next one
                last_active_screen = ui_advance_screen();
            } else if (event_data == BUTTON_LONG_PRESS){
                water_reset_stats();
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay
    }
}
