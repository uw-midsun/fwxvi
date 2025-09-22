#pragma once
#include <stddef.h>

typedef struct {
    float resistance; 
} ThermistorDataPoint;

extern const ThermistorDataPoint thermistor_table[];
extern const uint16_t thermistor_table_size;




