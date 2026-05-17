/************************************************************************************************
 * @file   hass50.c
 *
 * @brief  driver for the HASS50 current sensing driver
 *
 * @date   2025-01-28
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "log.h"
#include "status.h"

/* Intra-component Headers */
#include "hass50.h"

StatusCode hass50_init(HASS50Storage *storage, GpioAddress *out_pin, GpioAddress *ref_pin) {
  if (storage == NULL || out_pin == NULL || ref_pin == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->out_pin = *out_pin;
  storage->ref_pin = *ref_pin;

  StatusCode status = adc_add_channel(&storage->out_pin);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  status = adc_add_channel(&storage->ref_pin);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  return STATUS_CODE_OK;
}

StatusCode hass50_get_current(HASS50Storage *storage, float *current) {
  if (storage == NULL || current == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint16_t v_out_mv, v_ref_mv;
  StatusCode status = adc_read_converted(&storage->out_pin, &v_out_mv);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  status = adc_read_converted(&storage->ref_pin, &v_ref_mv);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  float v_out = v_out_mv / 1000.0f;
  float v_ref = v_ref_mv / 1000.0f;

  *current = (v_out - v_ref) * HASS50_IPN / HASS50_SENSITIVITY;

  return STATUS_CODE_OK;
}