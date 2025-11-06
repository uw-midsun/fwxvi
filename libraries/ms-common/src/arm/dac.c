/************************************************************************************************
 * @file   dac.c
 *
 * @brief  DAC Library Source code
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_dac.h"

/* Intra-component Headers */
#include "dac.h"

typedef struct DacChannelState {
  bool enabled;
  uint16_t current_value;
  SemaphoreHandle_t mutex_handle;
  StaticSemaphore_t mutex_buffer;
} DacChannelState;

typedef struct DacStatus {
  bool initialized;
  DacChannelState channels[NUM_DAC_CHANNELS];
} DacStatus;

/* DAC peripheral status */
static DacStatus s_dac_status = { 0 };

/* DAC1 handler */
static DAC_HandleTypeDef s_dac_handle;

/* Map DAC channel enum to HAL channel defines */
static uint32_t s_get_hal_channel(DacChannel channel) {
  return (channel == DAC_CHANNEL1) ? DAC_CHANNEL_1 : DAC_CHANNEL_2;
}

/* Validate channel parameter */
static StatusCode s_validate_channel(DacChannel channel) {
  if (channel >= NUM_DAC_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

/* Check if channel is enabled */
static StatusCode s_check_channel_enabled(DacChannel channel) {
  if (!s_dac_status.channels[channel].enabled) {
    return STATUS_CODE_UNINITIALIZED;
  }
  return STATUS_CODE_OK;
}

StatusCode dac_init(void) {
  if (s_dac_status.initialized) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Enable DAC clock */
  __HAL_RCC_DAC1_CLK_ENABLE();

  /* Initialize DAC peripheral */
  s_dac_handle.Instance = DAC1;

  if (HAL_DAC_Init(&s_dac_handle) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Initialize mutexes for each channel */
  for (uint8_t i = 0; i < NUM_DAC_CHANNELS; i++) {
    s_dac_status.channels[i].mutex_handle = xSemaphoreCreateMutexStatic(&s_dac_status.channels[i].mutex_buffer);

    if (s_dac_status.channels[i].mutex_handle == NULL) {
      return STATUS_CODE_INTERNAL_ERROR;
    }

    s_dac_status.channels[i].enabled = false;
    s_dac_status.channels[i].current_value = 0;
  }

  s_dac_status.initialized = true;

  return STATUS_CODE_OK;
}

StatusCode dac_enable_channel(DacChannel channel) {
  if (!s_dac_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(s_validate_channel(channel));

  if (s_dac_status.channels[channel].enabled) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  /* Configure DAC channel */
  DAC_ChannelConfTypeDef channel_config = { 0 };
  channel_config.DAC_Trigger = DAC_TRIGGER_NONE;
  channel_config.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

  uint32_t hal_channel = s_get_hal_channel(channel);

  if (HAL_DAC_ConfigChannel(&s_dac_handle, &channel_config, hal_channel) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Start DAC channel */
  if (HAL_DAC_Start(&s_dac_handle, hal_channel) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_dac_status.channels[channel].enabled = true;
  s_dac_status.channels[channel].current_value = 0;

  return STATUS_CODE_OK;
}

StatusCode dac_set_raw(DacChannel channel, uint16_t value) {
  if (!s_dac_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(s_validate_channel(channel));
  status_ok_or_return(s_check_channel_enabled(channel));

  if (value > DAC_MAX_VALUE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  DacChannelState *ch_state = &s_dac_status.channels[channel];
  uint32_t hal_channel = s_get_hal_channel(channel);

  /* Take mutex for thread safety */
  if (xSemaphoreTake(ch_state->mutex_handle, portMAX_DELAY) != pdTRUE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Set DAC output value (12-bit right-aligned) */
  if (HAL_DAC_SetValue(&s_dac_handle, hal_channel, DAC_ALIGN_12B_R, value) != HAL_OK) {
    xSemaphoreGive(ch_state->mutex_handle);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  ch_state->current_value = value;

  xSemaphoreGive(ch_state->mutex_handle);

  return STATUS_CODE_OK;
}

StatusCode dac_set_voltage(DacChannel channel, uint16_t voltage_mv) {
  if (voltage_mv > DAC_VREF_MV) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Convert voltage to raw DAC value */
  uint16_t raw_value = (voltage_mv * DAC_MAX_VALUE) / DAC_VREF_MV;

  return dac_set_raw(channel, raw_value);
}

StatusCode dac_get_raw(DacChannel channel, uint16_t *value) {
  if (value == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (!s_dac_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(s_validate_channel(channel));
  status_ok_or_return(s_check_channel_enabled(channel));

  DacChannelState *ch_state = &s_dac_status.channels[channel];

  /* Take mutex for thread safety */
  if (xSemaphoreTake(ch_state->mutex_handle, portMAX_DELAY) != pdTRUE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  *value = ch_state->current_value;

  xSemaphoreGive(ch_state->mutex_handle);

  return STATUS_CODE_OK;
}

StatusCode dac_get_voltage(DacChannel channel, uint16_t *voltage_mv) {
  if (voltage_mv == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint16_t raw_value;
  status_ok_or_return(dac_get_raw(channel, &raw_value));

  *voltage_mv = (raw_value * DAC_VREF_MV) / DAC_MAX_VALUE;

  return STATUS_CODE_OK;
}

StatusCode dac_disable_channel(DacChannel channel) {
  if (!s_dac_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(s_validate_channel(channel));

  if (!s_dac_status.channels[channel].enabled) {
    return STATUS_CODE_OK;
  }

  uint32_t hal_channel = s_get_hal_channel(channel);

  /* Stop DAC channel */
  if (HAL_DAC_Stop(&s_dac_handle, hal_channel) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_dac_status.channels[channel].enabled = false;
  s_dac_status.channels[channel].current_value = 0;

  return STATUS_CODE_OK;
}
