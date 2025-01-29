#pragma once

/************************************************************************************************
 * @file   aux_sense.h
 *
 * @brief  Source code for Aux Battery sense
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bms_carrier.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/**
 * @brief   Aux sense storage
 */
struct AuxSenseStorage {
    GpioAddress sense_adc;      /**< Aux sense analog pin */
    uint16_t batt_voltage_mv;   /**< Aux sense battery voltage */
};

/**
 * @brief   Initialize the aux sense interface
 * @param   storage Pointer to the BMS storage
 * @return  STATUS_CODE_OK if aux sense initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode aux_sense_init(BmsStorage *storage);

/**
 * @brief   Run the aux sense interface
 * @return  STATUS_CODE_OK if aux sense run succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode aux_sense_run(void);

/** @} */
