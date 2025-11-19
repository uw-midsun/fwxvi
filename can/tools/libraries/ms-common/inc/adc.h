#pragma once

/************************************************************************************************
 * @file   adc.h
 *
 * @brief  ADC Library Header file
 *
 * @date   2024-11-02
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
 * @defgroup ADC
 * @brief    ADC library
 * @{
 */

/** @brief  Total Number of ADC channels (16 external, 3 internal) */
#define NUM_ADC_CHANNELS 19U
/** @brief  Maximum time permitted for an ADC transaction */
#define ADC_TIMEOUT_MS 100U

/**
 * @brief   Available ADC modes
 */
typedef enum {
  ADC_MODE_SINGLE = 0, /**< Single mode */
  ADC_MODE_CONTINUOUS, /**< Continuous mode */
  NUM_ADC_MODES,       /**< Number of ADC Ports */
} AdcMode;

/**
 * @brief   Initialize the ADC instance
 * @details This shall configure the ADC for DMA transactions, increasing throughput
 *          ADC channels must be already added prior to calling this function
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 *          STATUS_CODE_RESOURCE_EXHAUSTED if already initialized
 */
StatusCode adc_init(void);

/**
 * @brief   Adds an ADC channel for a given Gpio
 * @details Configures a Gpio to be used as an ADC channel
 *          Must be called for all pins in use before adc_init
 * @param   address Specifies which Gpio to set as an ADC channel
 * @return  STATUS_CODE_OK if adding a channel succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if HAL initialization fails
 *          STATUS_CODE_RESOURCE_EXHAUSTED if already added
 */
StatusCode adc_add_channel(GpioAddress *address);

/**
 * @brief   Runs ADC conversions for all added channels
 * @details Uses DMA to perform the transaction
 * @return  STATUS_CODE_OK if conversions succeeded
 *          STATUS_CODE_INTERNAL_ERROR if HAL transaction fails
 *          STATUS_CODE_TIMEOUT if ADC conversions exceed ADC_TIMEOUT_MS
 */
StatusCode adc_run(void);

/**
 * @brief   Read the most recent raw value for a Gpio address
 * @param   address Specifies which Gpio to read from
 * @param   reading Pointer to a 16-bit value that will be updated with the reading
 * @return  STATUS_CODE_OK if reading succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if reading fails
 */
StatusCode adc_read_raw(GpioAddress *address, uint16_t *reading);

/**
 * @brief   Read the most recent converted value for a Gpio address in mV
 * @param   address Specifies which Gpio to read from
 * @param   reading Pointer to a 16-bit value that will be updated with the reading
 * @return  STATUS_CODE_OK if reading succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 *          STATUS_CODE_INTERNAL_ERROR if reading fails
 */
StatusCode adc_read_converted(GpioAddress *address, uint16_t *reading);

/**
 * @brief   Clears the ADC state
 */
void adc_deinit(void);

#ifdef MS_PLATFORM_X86

/**
 * @brief   Sets an ADC reading for a selected Gpio address
 * @param   address Specifies which Gpio to update
 * @param   reading Pointer to a 16-bit value that will be set for reading
 * @return  STATUS_CODE_OK if data is set successfully
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode adc_set_reading(GpioAddress *address, uint16_t reading);
#endif

/** @} */
