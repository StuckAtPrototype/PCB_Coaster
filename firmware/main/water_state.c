#include "water_state.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "weight_lookup.h"

#define MAX_READINGS 40
#define TEMPERATURE_MAX_READINGS 200
#define CUP_MIN_WEIGHT 100
#define CUP_REFILL_THRESHOLD 200
#define CUP_WEIGH_TIME_DELAY_US 5000000
#define REMINDER_INTERVAL_US (REMINDER_INTERVAL_MINUTES * 60UL * 1000000UL) // minutes in microseconds

static const char *TAG = "Water State";


static float weight_readings_buffer[MAX_READINGS];
static int weight_buffer_index = 0;
static uint8_t weight_buffer_initialized = 0;

static float temperature_readings_buffer[TEMPERATURE_MAX_READINGS];
static int temperature_buffer_index = 0;
static uint8_t temperature_buffer_initialized = 0;


WaterState water_state = {
        .sip_count = 0,
        .refill_count = 0,
        .last_event_time = 0,
        .time_elapsed_since_last_event = 0,
        .last_weight = 0,
        .weight_delta = 0,
        .weight_drift = 0,
        .cup_weigh_request = 0,
        .cup_temperature = 0,
        .drink_reminder = 0,
        .cup_state = CUP_INIT
};

void water_reset_stats(void){
    water_state.sip_count = 0;
    water_state.refill_count = 0;
    water_state.last_event_time = esp_timer_get_time();
}


void is_reminder_due(void) {
    if (water_state.last_event_time == 0) {
        return; // No reminder if no events have occurred yet
    }

    int64_t current_time = esp_timer_get_time();
    water_state.time_elapsed_since_last_event = current_time - water_state.last_event_time;

    // set the drink reminder if its time and the cup is present
    if (water_state.time_elapsed_since_last_event >= REMINDER_INTERVAL_US && water_state.cup_state == CUP_PRESENT){

//        ESP_LOGI(TAG, "Drink reminder at time %llu", current_time);

        // set the reminder flag
        water_state.drink_reminder = 1;

    }
}

// set the temperature variable
void add_temperature_reading(float temperature){
    water_state.cup_temperature = temperature;

    if (temperature_buffer_index < TEMPERATURE_MAX_READINGS) {
        temperature_readings_buffer[temperature_buffer_index++] = temperature;
    } else {
        // If full, shift left to remove the oldest reading.
        for (int i = 1; i < TEMPERATURE_MAX_READINGS; i++) {
            temperature_readings_buffer[i - 1] = temperature_readings_buffer[i];
    }
        temperature_readings_buffer[TEMPERATURE_MAX_READINGS - 1] = temperature;

        // we have reached a full buffer
        temperature_buffer_initialized = 1;
    }
}

float temperature_average(void){
    float avg_temperature = 0.0f;

    // only proceed with the average if we are initialized
    if(temperature_buffer_initialized == 1){
        for(int i = 0; i < MAX_READINGS; i++){
            avg_temperature += temperature_readings_buffer[i];
        }
        avg_temperature /= MAX_READINGS;
    }
    return avg_temperature;
}

// Add a new reading to the buffer one element at a time.
// If the buffer is full, we shift the data to make room for new readings.
void add_weight_reading(float new_reading)
{
    if (weight_buffer_index < MAX_READINGS) {
        weight_readings_buffer[weight_buffer_index++] = new_reading;
    } else {
        // If full, shift left to remove the oldest reading.
        for (int i = 1; i < MAX_READINGS; i++) {
            weight_readings_buffer[i - 1] = weight_readings_buffer[i];
        }
        weight_readings_buffer[MAX_READINGS - 1] = new_reading;

        // we have reached a full buffer
        weight_buffer_initialized = 1;
    }
}

float weight_average(void){
    float avg_weight = 0.0f;

    // only proceed with the average if we are initialized
    if(weight_buffer_initialized == 1){
        for(int i = 0; i < MAX_READINGS; i++){
            avg_weight += weight_readings_buffer[i];
        }
        avg_weight /= MAX_READINGS;
    }
    return avg_weight;
}

/* MVP
 * Measure how many sips we took
 * Track temperature
 * Set notification request
 * Track refills
 */
void process_cup(void){

    // not a fan of this as we should have a single return point
    // but i am taking a shortcut for now
    if(weight_buffer_initialized == 0)
        return;

    float weight = weight_average();
    // process transition points
    // only initialize if not initialized before
    if(water_state.cup_state == CUP_INIT){

        if(weight < CUP_MIN_WEIGHT) {
            water_state.cup_state = CUP_NOT_PRESENT;
            water_state.last_event_time = esp_timer_get_time();
        }
        else {
            water_state.cup_state = CUP_PRESENT;
            water_state.last_event_time = esp_timer_get_time();
            water_state.last_weight = weight;
            ESP_LOGI(TAG, "Cup init, weight:%.0f", weight);
        }
    } else if(weight < CUP_MIN_WEIGHT && water_state.cup_state == CUP_PRESENT){

        // we just took off the cup
        // clear states and record event
        water_state.drink_reminder = 0;
        water_state.cup_state = CUP_NOT_PRESENT;
        water_state.sip_count++;
        water_state.last_event_time = esp_timer_get_time();

        ESP_LOGI(TAG, "Cup has been lifted, weight_readings_buffer: %.0f last_weight:%.0f",
                 weight_readings_buffer[0], water_state.last_weight);

        // compensate the drift
        water_state.last_weight += (water_state.last_weight - weight_readings_buffer[0]);


    } else if(weight >= CUP_MIN_WEIGHT && water_state.cup_state == CUP_NOT_PRESENT){
        water_state.cup_state = CUP_PRESENT;
        water_state.last_event_time = esp_timer_get_time();
        water_state.cup_weigh_request = 1;
        ESP_LOGI(TAG, "Cup has been replaced, time: %llu", water_state.last_event_time);
    }

    // process cup weighing
    if(water_state.cup_weigh_request == 1 && water_state.cup_state == CUP_PRESENT) {
        if ((esp_timer_get_time() - water_state.last_event_time) > CUP_WEIGH_TIME_DELAY_US) {

            ESP_LOGI(TAG, "Cup weight: %.0f last_weight: %.0f", weight, water_state.last_weight);

            // update the refill count
            if((weight - water_state.last_weight) > CUP_REFILL_THRESHOLD){
                water_state.refill_count++;
                ESP_LOGI(TAG, "Cup has been refilled: %i", water_state.refill_count);
            }

            // update the vars
            water_state.weight_delta = water_state.last_weight - weight;
            water_state.last_weight = weight;
            water_state.cup_weigh_request = 0;
        }
    }

    // track the weight drift
    if(water_state.cup_state == CUP_PRESENT){
        water_state.weight_drift = water_state.last_weight - weight;
    }

    // track water intake timing
    is_reminder_due();

}