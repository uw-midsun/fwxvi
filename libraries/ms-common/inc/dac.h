#pragma once

/************************************************************************************************
 * @file   dac.h
 *
 * @brief  DAC Library Header file
 *
 * @date   2025-05-10
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/**
 * @defgroup DAC
 * @brief    DAC library
 * @{
 */

/**
 * @brief   DAC Output Channel
 */
typedef enum {
  MS_DAC_CHANNEL_1 = 0, /**< DAC output channel 1 */
  MS_DAC_CHANNEL_2,     /**< DAC output channel 2 */
  NUM_DAC_CHANNELS,  /**< Total number of DAC channels */
} DacChannel;

/**
 * @brief   DAC Configuration struct
 */
typedef struct {
  DacChannel channel;   /**< Channel to configure */
  bool enable_buffer;   /**< Enable output buffer */
  bool enable_trigger;  /**< Enable trigger mode */
} DacConfig;

/**
 * @brief   Initialize a DAC output channel
 * @param   config Pointer to DacConfig containing channel and config options
 * @return  STATUS_CODE_OK if initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if input is NULL or invalid
 *          STATUS_CODE_INTERNAL_ERROR if DAC setup fails
 */
StatusCode dac_init(const DacConfig *config);

/**
 * @brief   Set DAC output value for a specific channel
 * @param   channel Channel to write to
 * @param   value 12-bit digital value (0 - 4095)
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if channel or value is invalid
 */
StatusCode dac_set_value(DacChannel channel, uint16_t value);

/**
 * @brief   Deinitialize the DAC (optional cleanup)
 */
void dac_deinit(void);

/** @} */