/************************************************************************************************
 * @file   opamp.c
 *
 * @brief  OPAMP Library Source code
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
#include "stm32l4xx_hal_opamp.h"

/* Intra-component Headers */
#include "opamp.h"

typedef struct OpampState {
  bool configured;
  bool running;
  OpampConfig config;
  SemaphoreHandle_t mutex_handle;
  StaticSemaphore_t mutex_buffer;
} OpampState;

typedef struct OpampStatus {
  bool initialized;
  OpampState instances[NUM_OPAMP_INSTANCES];
} OpampStatus;

/* OPAMP peripheral status */
static OpampStatus s_opamp_status = { 0 };

/* OPAMP1 handler */
static OPAMP_HandleTypeDef s_opamp_handle;

/* Get HAL OPAMP instance */
static OPAMP_TypeDef *s_get_hal_instance(OpampInstance instance) {
  return OPAMP1; /* STM32L433 only has OPAMP1 */
}

/* Validate instance parameter */
static StatusCode s_validate_instance(OpampInstance instance) {
  if (instance >= NUM_OPAMP_INSTANCES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  return STATUS_CODE_OK;
}

/* Map VINP selection to HAL defines */
static uint32_t s_get_hal_vinp(OpampNonInvertingInput vinp) {
  switch (vinp) {
    case OPAMP_NONINVERTING_IO0:
      return OPAMP_NONINVERTINGINPUT_IO0; /* PA1 */
    case OPAMP_NONINVERTING_DAC:
      return OPAMP_NONINVERTINGINPUT_DAC_CH;
    default:
      return OPAMP_NONINVERTINGINPUT_IO0;
  }
}

/* Map VINM selection to HAL defines */
static uint32_t s_get_hal_vinm(OpampInvertingInput vinm) {
  switch (vinm) {
    case OPAMP_INVERTING_IO0:
      return OPAMP_INVERTINGINPUT_IO0; /* PA0 - External feedback */
    case OPAMP_INVERTING_IO1:
      return OPAMP_INVERTINGINPUT_IO1; /* PC5 - External feedback */
    case OPAMP_INVERTING_PGA:
      return OPAMP_INVERTINGINPUT_IO0; /* PGA mode uses IO0 */
    case OPAMP_INVERTING_FOLLOWER:
      return OPAMP_INVERTINGINPUT_IO0; /* Follower - VINM unused */
    default:
      return OPAMP_INVERTINGINPUT_IO0;
  }
}

/* Map PGA gain to HAL defines */
static uint32_t s_get_hal_pga_gain(OpampProgrammableGain gain) {
  switch (gain) {
    case OPAMP_PROGRAMMABLE_GAIN_2:
      return OPAMP_PGA_GAIN_2;
    case OPAMP_PROGRAMMABLE_GAIN_4:
      return OPAMP_PGA_GAIN_4;
    case OPAMP_PROGRAMMABLE_GAIN_8:
      return OPAMP_PGA_GAIN_8;
    case OPAMP_PROGRAMMABLE_GAIN_16:
      return OPAMP_PGA_GAIN_16;
    case OPAMP_PROGRAMMABLE_GAIN_2_INVERTING:
      return OPAMP_PGA_GAIN_2;
    case OPAMP_PROGRAMMABLE_GAIN_4_INVERTING:
      return OPAMP_PGA_GAIN_4;
    case OPAMP_PROGRAMMABLE_GAIN_8_INVERTING:
      return OPAMP_PGA_GAIN_8;
    case OPAMP_PROGRAMMABLE_GAIN_16_INVERTING:
      return OPAMP_PGA_GAIN_16;
    default:
      return OPAMP_PGA_GAIN_2;
  }
}

/* Get OPAMP mode based on configuration */
static uint32_t s_get_hal_mode(OpampInvertingInput vinm) {
  switch (vinm) {
    case OPAMP_INVERTING_PGA:
      return OPAMP_PGA_MODE;
    case OPAMP_INVERTING_FOLLOWER:
      return OPAMP_FOLLOWER_MODE;
    default:
      return OPAMP_STANDALONE_MODE; /* External feedback */
  }
}

StatusCode opamp_init(void) {
  if (s_opamp_status.initialized) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Enable OPAMP clock */
  __HAL_RCC_OPAMP_CLK_ENABLE();

  /* Initialize mutexes for each instance */
  for (uint8_t i = 0; i < NUM_OPAMP_INSTANCES; i++) {
    s_opamp_status.instances[i].mutex_handle = xSemaphoreCreateMutexStatic(&s_opamp_status.instances[i].mutex_buffer);

    if (s_opamp_status.instances[i].mutex_handle == NULL) {
      return STATUS_CODE_INTERNAL_ERROR;
    }

    s_opamp_status.instances[i].configured = false;
    s_opamp_status.instances[i].running = false;
  }

  s_opamp_status.initialized = true;

  return STATUS_CODE_OK;
}

StatusCode opamp_configure(OpampInstance instance, OpampConfig *config) {
  if (!s_opamp_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(s_validate_instance(instance));

  OpampState *state = &s_opamp_status.instances[instance];

  /* Take mutex for thread safety */
  if (xSemaphoreTake(state->mutex_handle, portMAX_DELAY) != pdTRUE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Stop OPAMP if it's running */
  if (state->running) {
    HAL_OPAMP_Stop(&s_opamp_handle);
    state->running = false;
  }

  /* Configure OPAMP based on actual STM32L433 HAL structure */
  s_opamp_handle.Instance = s_get_hal_instance(instance);
  s_opamp_handle.Init.PowerSupplyRange = OPAMP_POWERSUPPLY_HIGH; /* Above 2.4V */
  s_opamp_handle.Init.PowerMode = OPAMP_POWERMODE_NORMAL;
  s_opamp_handle.Init.Mode = s_get_hal_mode(config->vinm_sel);
  s_opamp_handle.Init.NonInvertingInput = s_get_hal_vinp(config->vinp_sel);
  s_opamp_handle.Init.InvertingInput = s_get_hal_vinm(config->vinm_sel);

  /* Configure PGA gain if in PGA mode */
  if (config->vinm_sel == OPAMP_INVERTING_PGA) {
    s_opamp_handle.Init.PgaGain = s_get_hal_pga_gain(config->pga_gain);
  } else {
    s_opamp_handle.Init.PgaGain = OPAMP_PGA_GAIN_2; /* Default, not used */
  }

  /* Use factory trimming by default */
  s_opamp_handle.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  s_opamp_handle.Init.TrimmingValueP = 16; /* Default trimming values */
  s_opamp_handle.Init.TrimmingValueN = 16;
  s_opamp_handle.Init.TrimmingValuePLowPower = 16;
  s_opamp_handle.Init.TrimmingValueNLowPower = 16;

  if (HAL_OPAMP_Init(&s_opamp_handle) != HAL_OK) {
    xSemaphoreGive(state->mutex_handle);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  /* Store configuration */
  state->config = *config;
  state->configured = true;

  xSemaphoreGive(state->mutex_handle);

  return STATUS_CODE_OK;
}

StatusCode opamp_start(OpampInstance instance) {
  if (!s_opamp_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(s_validate_instance(instance));

  OpampState *state = &s_opamp_status.instances[instance];

  if (!state->configured) {
    return STATUS_CODE_UNINITIALIZED;
  }

  /* Take mutex for thread safety */
  if (xSemaphoreTake(state->mutex_handle, portMAX_DELAY) != pdTRUE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (!state->running) {
    if (HAL_OPAMP_Start(&s_opamp_handle) != HAL_OK) {
      xSemaphoreGive(state->mutex_handle);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    state->running = true;
  }

  xSemaphoreGive(state->mutex_handle);

  return STATUS_CODE_OK;
}

StatusCode opamp_stop(OpampInstance instance) {
  if (!s_opamp_status.initialized) {
    return STATUS_CODE_UNINITIALIZED;
  }

  status_ok_or_return(s_validate_instance(instance));

  OpampState *state = &s_opamp_status.instances[instance];

  /* Take mutex for thread safety */
  if (xSemaphoreTake(state->mutex_handle, portMAX_DELAY) != pdTRUE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (state->running) {
    if (HAL_OPAMP_Stop(&s_opamp_handle) != HAL_OK) {
      xSemaphoreGive(state->mutex_handle);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    state->running = false;
  }

  xSemaphoreGive(state->mutex_handle);

  return STATUS_CODE_OK;
}
