/************************************************************************************************
 * @file   adc.c
 *
 * @brief  ADC Library Source code
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <string.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "delay.h"
#include "semphr.h"

/* Intra-component Headers */
#include "adc.h"

#define VREFINT_MV 3300U
#define ADC_MAX_VAL 4095U
#define MAX_ADC_READINGS 16U

typedef struct AdcStatus {
  bool initialized;
  uint8_t active_channels;
  SemaphoreHandle_t converting_handle;
  StaticSemaphore_t converting_semaphore;
} AdcStatus;

/* ADC1 peripheral status */
static AdcStatus s_adc_status;

/* Array of readings, in order of rank, which DMA will write to */
static volatile uint16_t s_adc_readings[MAX_ADC_READINGS];

/* Store of ranks for each channel, at their channel index */
static uint8_t s_adc_ranks[NUM_ADC_CHANNELS];

/* Convert GPIO address to one of the 16 ADC channels */
/* Channels 1-4 are occupied by pins C0-C3 */
/* Channels 5-12 are occupied by pins A0-A7 */
/* Channels 13-14 are occupied by pins C4-C5 */
/* Channels 15-16 are occupied by pins B0-B1 */
static StatusCode s_adc_get_channel(GpioAddress *address, uint8_t *adc_channel) {
  *adc_channel = address->pin;
  switch (address->port) {
    case GPIO_PORT_A:
      if (address->pin > 7U) {
        return STATUS_CODE_INVALID_ARGS;
      }
      *adc_channel += 5U;
      break;
    case GPIO_PORT_B:
      if (address->pin > 1U) {
        return STATUS_CODE_INVALID_ARGS;
      }
      *adc_channel += 15U;
      break;
    case GPIO_PORT_C:
      if (address->pin > 5U) {
        return STATUS_CODE_INVALID_ARGS;
      }
      if (address->pin < 4U) {
        *adc_channel += 1U;
      } else {
        *adc_channel += 9U;
      }
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  if (*adc_channel < 1U || *adc_channel > NUM_ADC_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

static StatusCode s_check_channel_enabled(uint8_t channel) {
  if (s_adc_ranks[channel] != 0U) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_UNINITIALIZED;
  }
}

static void s_adc_mock_reading(void) {
  delay_ms(25);
  xSemaphoreGive(s_adc_status.converting_handle);
}

StatusCode adc_init(void) {
  s_adc_status.converting_handle = xSemaphoreCreateBinaryStatic(&s_adc_status.converting_semaphore);

  if (s_adc_status.converting_handle == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode adc_add_channel(GpioAddress *address) {
  if (address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_adc_status.initialized) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  uint8_t channel;
  status_ok_or_return(s_adc_get_channel(address, &channel));

  if (s_check_channel_enabled(channel) == STATUS_CODE_OK) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  if (++s_adc_status.active_channels < NUM_ADC_CHANNELS) {
    s_adc_ranks[channel] = s_adc_status.active_channels;
  } else {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  return STATUS_CODE_OK;
}

StatusCode adc_run(void) {
  s_adc_mock_reading();

  if (xSemaphoreTake(s_adc_status.converting_handle, pdMS_TO_TICKS(ADC_TIMEOUT_MS)) != pdTRUE) {
    return STATUS_CODE_TIMEOUT;
  }

  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_read_raw(GpioAddress *address, uint16_t *reading) {
  if (reading == NULL || address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t channel;
  status_ok_or_return(s_adc_get_channel(address, &channel));
  status_ok_or_return(s_check_channel_enabled(channel));

  /* Index of reading is its rank - 1 */
  *reading = s_adc_readings[s_adc_ranks[channel] - 1U];

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(GpioAddress *address, uint16_t *reading) {
  if (reading == NULL || address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t raw_value;
  adc_read_raw(address, &raw_value);

  *reading = (raw_value * VREFINT_MV) / ADC_MAX_VAL;

  return STATUS_CODE_OK;
}

void adc_deinit(void) {
  memset(&s_adc_status, 0, sizeof(s_adc_status));
  memset(s_adc_readings, 0, sizeof(uint16_t) * MAX_ADC_READINGS);
  memset(s_adc_ranks, 0, sizeof(uint8_t) * NUM_ADC_CHANNELS);
}

StatusCode adc_set_reading(GpioAddress *address, uint16_t reading) {
  if (address == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t channel;
  status_ok_or_return(s_adc_get_channel(address, &channel));
  status_ok_or_return(s_check_channel_enabled(channel));

  /* We will convert our passed value in mV to a raw reading */
  s_adc_readings[s_adc_ranks[channel] - 1U] = (reading * ADC_MAX_VAL) / VREFINT_MV;
  return STATUS_CODE_OK;
}
