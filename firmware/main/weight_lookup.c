#include "weight_lookup.h"

typedef struct {
    int input;
    float output;
} LookupEntry;

const LookupEntry lookup_table[] = {
        {500, 59.0},
        {570, 71.4},
        {720, 116.2},
        {908, 161.5},
        {1100, 192.5},
        {1670, 366.0},
        {1720, 401.8},
        {1950, 532.6},
        {2130, 637.8},
        {2170, 705.4}
};

static const int table_size = sizeof(lookup_table) / sizeof(LookupEntry);

float lookup_and_extrapolate(int input) {
    int i;
    for (i = 0; i < table_size - 1; i++) {
        if (input <= lookup_table[i + 1].input) {
            float x0 = lookup_table[i].input;
            float y0 = lookup_table[i].output;
            float x1 = lookup_table[i + 1].input;
            float y1 = lookup_table[i + 1].output;
            return y0 + (y1 - y0) * (input - x0) / (x1 - x0);
        }
    }

    // Extrapolate beyond the last entry
    float x0 = lookup_table[table_size - 2].input;
    float y0 = lookup_table[table_size - 2].output;
    float x1 = lookup_table[table_size - 1].input;
    float y1 = lookup_table[table_size - 1].output;
    return y0 + (y1 - y0) * (input - x0) / (x1 - x0);
}