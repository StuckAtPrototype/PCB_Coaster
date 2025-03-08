#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define BUTTON_PIN 10  // this is the IO # we need
#define LONG_PRESS_TIME_MICROS 1000 * 1000   // 1 second for long press in Micro Seconds

typedef enum {
    BUTTON_LONG_PRESS,
    BUTTON_SHORT_PRESS,
    BUTTON_NONE
} button_event_t;

typedef enum {
    BUTTON_LOW,
    BUTTON_HIGH
} button_state_t;


QueueHandle_t button_get_queue_handle(void);
void button_handler_init(void);


#endif // BUTTON_HANDLER_H
