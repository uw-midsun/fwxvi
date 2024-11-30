#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "notify.h"
#include "status.h"
#include "tasks.h"


//total # of channels, 16 external, 5 internal
#define NUM_ADC_CHANNELS 21
#define ADC_TIMEOUT_MS 100

typedef enum{
    ADC_MODE_SINGLE = 0,
    ADC_MODE_CONTINUOUS,
    NUM_ADC_MODES
} AdcMode;

extern const GpioAddress ADC_REF;
extern const GpioAddress ADC_TEMP;

StatusCode adc_init(void);
StatusCode adc_add_channel(GpioAddress address);
