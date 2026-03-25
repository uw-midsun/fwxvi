#pragma once

/************************************************************************************************
 * @file   dac.h
 *
 * @brief  DAC Library Header
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"
#include "status.h"

/**
 * @defgroup DAC
 * @brief    DAC library
 * @{
 */

/* Number of DAC channels on STM32L433 (DAC1 has 2 channels) */
#define NUM_DAC_CHANNELS 2U

/* DAC resolution - 12-bit */
#define DAC_MAX_VALUE 4095U

/* Reference voltage in mV (typically 3.3V) */
#define DAC_VREF_MV 3300U

/**
 * @brief   DAC channel identifiers
 */
typedef enum {
  DAC_CHANNEL1 = 0, /**< DAC Channel 1 (PA4) */
  DAC_CHANNEL2 = 1, /**< DAC Channel 2 (PA5) */
} DacChannel;

/**
 * @brief   Initialize the DAC peripheral
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_RESOURCE_EXHAUSTED if already initialized
 */
StatusCode dac_init(void);

/**
 * @brief   Enable a specific DAC channel
 * @param   channel DAC channel to enable (DAC_CHANNEL_1 or DAC_CHANNEL_2)
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if channel is invalid
 *          STATUS_CODE_UNINITIALIZED if DAC not initialized
 *          STATUS_CODE_RESOURCE_EXHAUSTED if channel already enabled
 */
StatusCode dac_enable_channel(DacChannel channel);

/**
 * @brief   Set DAC output using raw 12-bit value
 * @param   channel DAC channel to set
 * @param   value Raw 12-bit value (0-4095)
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if channel invalid or value out of range
 *          STATUS_CODE_UNINITIALIZED if DAC not initialized or channel not enabled
 */
StatusCode dac_set_raw(DacChannel channel, uint16_t value);

/**
 * @brief   Set DAC output using millivolt value
 * @param   channel DAC channel to set
 * @param   voltage_mv Voltage in millivolts (0-3300)
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if channel invalid or voltage out of range
 *          STATUS_CODE_UNINITIALIZED if DAC not initialized or channel not enabled
 */
StatusCode dac_set_voltage(DacChannel channel, uint16_t voltage_mv);

/**
 * @brief   Read back the current DAC setting (raw value)
 * @param   channel DAC channel to read
 * @param   value Pointer to store the current raw value
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if parameters are invalid
 *          STATUS_CODE_UNINITIALIZED if DAC not initialized or channel not enabled
 */
StatusCode dac_get_raw(DacChannel channel, uint16_t *value);

/**
 * @brief   Read back the current DAC setting (voltage in mV)
 * @param   channel DAC channel to read
 * @param   voltage_mv Pointer to store the current voltage in millivolts
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if parameters are invalid
 *          STATUS_CODE_UNINITIALIZED if DAC not initialized or channel not enabled
 */
StatusCode dac_get_voltage(DacChannel channel, uint16_t *voltage_mv);

/**
 * @brief   Disable a specific DAC channel
 * @param   channel DAC channel to disable
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if channel is invalid
 *          STATUS_CODE_UNINITIALIZED if DAC not initialized
 */
StatusCode dac_disable_channel(DacChannel channel);

/** @} */
