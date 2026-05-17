#pragma once

/************************************************************************************************
 * @file    hass50.h
 *
 * @brief   Header file to implement the current sensing from the HASS50 sensor
 *
 * @date    2026-05-17
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"

/* Intra-component Headers */
#include "status.h"

#define HASS50_IPN 50.0f
#define HASS50_UREF 1.8f
#define HASS50_SENSITIVITY 0.625f

/**
 * @defgroup HASS50 Current Sensing
 * @brief    HASS50 Current Sensing library
 * @{
 */
typedef struct {
  GpioAddress out_pin;
  GpioAddress ref_pin;
} HASS50Storage;

/**
 * @brief Initialize the HASS50 driver
 * @param storage - a pointer to the HASS50 struct, to be initialized
 * @param out_pin - GPIO pin connected to the HASS50 output
 * @param ref_pin - GPIO pin connected to the HASS50 reference voltage
 * @return STATUS_CODE_OK on success
 */
StatusCode hass50_init(HASS50Storage *storage, GpioAddress *out_pin, GpioAddress *ref_pin);

/**
 * @brief Gets the instantaneous current in Amps
 * @param storage - pointer to already initialized HASS50 struct
 * @param out_current_amps - current in amps
 * @return STATUS_CODE_OK on success
 */
StatusCode hass50_get_current(HASS50Storage *storage, float *out_current_amps);

/** @} */
