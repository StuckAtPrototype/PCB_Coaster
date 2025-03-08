#include "button_handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "BUTTON_HANDLER";

QueueHandle_t gpio_evt_queue = NULL;
static TaskHandle_t button_task_handle = NULL;


typedef struct {
    uint64_t last_event_time;
    button_state_t last_button_state;
    button_event_t button_event;
    uint8_t button_event_started;
} ButtonStateMachine;

// button state machine
ButtonStateMachine button_sm = {
        .last_event_time = 0,
        .last_button_state = BUTTON_LOW,
        .button_event = BUTTON_NONE,
        .button_event_started = 0
};

QueueHandle_t button_get_queue_handle(void) {
    return gpio_evt_queue;
}

static void button_task(void* arg)
{
    button_event_t event;
    while(1) {
        // Check if it's a high or low
        if (gpio_get_level(BUTTON_PIN) == 1) {

            // check if we are currently in a event started mode and if the last button state was low
            if (button_sm.button_event_started == 0 && button_sm.last_button_state == BUTTON_LOW) {
                button_sm.last_event_time = esp_timer_get_time();
                button_sm.button_event_started = 1;
            } else if (button_sm.button_event_started == 1){
                // exit point for a long press
                uint64_t time_now = esp_timer_get_time();
                if(time_now - button_sm.last_event_time >= LONG_PRESS_TIME_MICROS){
                    event = BUTTON_LONG_PRESS;
                    ESP_LOGI(TAG, "Long press detected");

                    // clean up the state
                    button_sm.button_event_started = 0;

                    // send out the queue to the rest of the system
                    xQueueSend(gpio_evt_queue, &event, 0);
                }
            }
            // mark the state machine as button state high
            button_sm.last_button_state = BUTTON_HIGH;

        } else {
            // if the event is started and the last button state is high
            if(button_sm.button_event_started == 1 && button_sm.last_button_state == BUTTON_HIGH){

                // if time is greater than limit, post event as long press and stop and reset timer
                uint64_t time_now = esp_timer_get_time();
                if (time_now - button_sm.last_event_time <= LONG_PRESS_TIME_MICROS) {
                    ESP_LOGI(TAG, "Short press detected");
                    event = BUTTON_SHORT_PRESS;
                }
                // clean up the state
                button_sm.button_event_started = 0;

                // send out the queue to the rest of the system
                xQueueSend(gpio_evt_queue, &event, 0);
            }
            // mark the state machine as button state high
            button_sm.last_button_state = BUTTON_LOW;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void button_handler_init(void)
{
    gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << BUTTON_PIN),
            .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, &button_task_handle);

    ESP_LOGI(TAG, "Button handler initialized");
}
