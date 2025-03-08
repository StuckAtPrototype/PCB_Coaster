#ifndef WATER_STATE_H
#define WATER_STATE_H

#include <stdint.h>

#define REMINDER_INTERVAL_MINUTES 15 // modify this value

enum CupState {
    CUP_INIT,
    CUP_PRESENT,
    CUP_NOT_PRESENT
};

typedef struct {
    uint16_t sip_count;         // how many times we have sipped water
    uint16_t refill_count;      // how many times we have refilled water
    uint64_t last_event_time;   // what time the last transition event happened
    uint64_t time_elapsed_since_last_event;   // how much time elapsed since last event
    float last_weight;          // last recorded weight
    float weight_delta;         // the delta between last recorded weight and weight now
    float weight_drift;         // how much drift we are seeing since last recorded weight without a cup lift
    uint8_t cup_weigh_request;  // request for a cup weigh
    float cup_temperature;      // cup temperature (internal ESP32 temp)
    uint8_t drink_reminder;     // drink reminder flag
    enum CupState cup_state;    // initialize, cup on coaster, cup not on coaster
} WaterState;

extern WaterState water_state;


// resets statistics
void water_reset_stats(void);

// adds a temperature reading
void add_temperature_reading(float temperature);

// adds a reading to the moving buffer array
void add_weight_reading(float new_reading);

// gives us the weight average of the buffer array
// returns 0 if not fully initialized
float weight_average(void);

// returns the average temperature
float temperature_average(void);

// processes the state machine for sip counting and weighing the cup
void process_cup(void);

#endif // WATER_STATE_H
