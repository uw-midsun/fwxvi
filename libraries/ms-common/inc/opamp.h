#pragma once

/************************************************************************************************
 * @file   opamp.h
 *
 * @brief  OPAMP Library Header - Internal Op-Amp Driver for STM32L433
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

/**
 * @defgroup OpAmp
 * @brief    OpAmp library
 * @{
 */

/**
 * @brief   OPAMP instance identifiers
 */
typedef enum {
  OPAMP_1 = 0, /**< OPAMP1 - PA1 (VINP), PA3 (VOUT), Internal VINM options */
  NUM_OPAMP_INSTANCES
} OpampInstance;

/**
 * @brief   OPAMP non-inverting input selection (VINP)
 */
typedef enum {
  OPAMP_NONINVERTING_IO0 = 0, /**< PA1 for OPAMP1 */
  OPAMP_NONINVERTING_DAC,     /**< DAC channel output (internal) */
} OpampNonInvertingInput;

/**
 * @brief   OPAMP inverting input selection (VINM)
 */
typedef enum {
  OPAMP_INVERTING_IO0 = 0,  /**< PA0 for OPAMP1 - External feedback */
  OPAMP_INVERTING_IO1,      /**< PC5 for OPAMP1 - External feedback */
  OPAMP_INVERTING_PGA,      /**< PGA mode with internal resistors */
  OPAMP_INVERTING_FOLLOWER, /**< Follower mode (VINM connected to VOUT internally) */
} OpampInvertingInput;

/**
 * @brief   OPAMP PGA gain configuration (when using PGA mode)
 */
typedef enum {
  OPAMP_PROGRAMMABLE_GAIN_2 = 0,        /**< Gain = 2 (Non-inverting gain) */
  OPAMP_PROGRAMMABLE_GAIN_4,            /**< Gain = 4 */
  OPAMP_PROGRAMMABLE_GAIN_8,            /**< Gain = 8 */
  OPAMP_PROGRAMMABLE_GAIN_16,           /**< Gain = 16 */
  OPAMP_PROGRAMMABLE_GAIN_2_INVERTING,  /**< Gain = -1 (Inverting with external R) */
  OPAMP_PROGRAMMABLE_GAIN_4_INVERTING,  /**< Gain = -3 (Inverting with external R) */
  OPAMP_PROGRAMMABLE_GAIN_8_INVERTING,  /**< Gain = -7 (Inverting with external R) */
  OPAMP_PROGRAMMABLE_GAIN_16_INVERTING, /**< Gain = -15 (Inverting with external R) */
} OpampProgrammableGain;

/**
 * @brief   OPAMP configuration structure
 */
typedef struct {
  OpampNonInvertingInput vinp_sel; /**< Non-inverting input selection */
  OpampInvertingInput vinm_sel;    /**< Inverting input selection */
  OpampProgrammableGain pga_gain;  /**< PGA gain (only used if vinm_sel = OPAMP_INVERTING_PGA) */
  bool output_to_adc;              /**< Route output internally to ADC */
} OpampConfig;

/**
 * @brief   Initialize the OPAMP peripheral
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if already initialized
 */
StatusCode opamp_init(void);

/**
 * @brief   Configure and start an OPAMP instance
 * @param   instance OPAMP instance to configure
 * @param   config Pointer to configuration structure
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if parameters are invalid
 *          STATUS_CODE_UNINITIALIZED if OPAMP not initialized
 *          STATUS_CODE_INTERNAL_ERROR if HAL operation fails
 */
StatusCode opamp_configure(OpampInstance instance, OpampConfig *config);

/**
 * @brief   Start an OPAMP instance (enable output)
 * @param   instance OPAMP instance to start
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if instance is invalid
 *          STATUS_CODE_UNINITIALIZED if OPAMP not initialized or not configured
 */
StatusCode opamp_start(OpampInstance instance);

/**
 * @brief   Stop an OPAMP instance (disable output)
 * @param   instance OPAMP instance to stop
 * @return  STATUS_CODE_OK if successful
 *          STATUS_CODE_INVALID_ARGS if instance is invalid
 *          STATUS_CODE_UNINITIALIZED if OPAMP not initialized
 */
StatusCode opamp_stop(OpampInstance instance);

/** @} */
