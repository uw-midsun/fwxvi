#pragma once

/************************************************************************************************
 * @file   adc.h
 *
 * @brief  Header file for the adc library
 *
 * @date   2024-12-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/


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

StatusCode adc_get_channel(GpioAddress address, uint8_t *adc_channel);
static uint16_t prv_get_temp(uint16_t *reading);
static StatusCode prv_check_channel_enabled(uint8_t channel);
StatusCode adc_add_channel(GpioAddress address);
StatusCode adc_init(void);
void adc_deint(void);
static void prv_adc_mock_reading(void);
StatusCode adc_run(void);
StatusCode adc_read_raw(GpioAddress address, uint16_t *reading);
StatusCode adc_read_converted(GpioAddress address, uint16_t *reading);
StatusCode adc_set_reading(GpioAddress sample_address, uint16_t adc_reading);
