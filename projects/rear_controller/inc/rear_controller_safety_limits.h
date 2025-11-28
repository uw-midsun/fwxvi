#pragma once

/************************************************************************************************
 * @file   rear_controller_safety_limits.h
 *
 * @brief  Safety-critical hard limits for Rear Controller
 *
 * @date   2025-05-28
 * @author Midnight Sun Team
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

/************************************************************************************************
 * Pack Current Limits [mA]
 ************************************************************************************************/
#define PACK_MAX_DISCHARGE_CURRENT_A 80.0  /**< Maximum allowed discharge current */
#define PACK_MAX_CHARGE_CURRENT_A 80.0     /**< Maximum allowed charge current */
#define PACK_SHORT_CIRCUIT_CURRENT_A 60000 /**< Instantaneous short-circuit threshold */

/************************************************************************************************
 * Voltage Limits [mV]
 ************************************************************************************************/
#define CELL_OVERVOLTAGE_LIMIT_mV 4200  /**< Over-voltage cutoff */
#define CELL_UNDERVOLTAGE_LIMIT_mV 2500 /**< Under-voltage cutoff */
#define CELL_UNBALANCED_LIMIT_mV 500    /**< Cell imbalance cutoff */
#define PACK_OVERVOLTAGE_LIMIT_mV 151200
#define PACK_UNDERVOLTAGE_LIMIT_mV 90000

/************************************************************************************************
 * Temperature Limits [°C]
 ************************************************************************************************/
#define CELL_OVERTEMP_DISCHARGE_LIMIT_C 60 /**< Max temperature for discharge */
#define CELL_OVERTEMP_CHARGE_LIMIT_C 60    /**< Max temperature for charge */
#define CELL_UNDERTEMP_CHARGE_LIMIT_C 0    /**< Min temperature for charge */

/************************************************************************************************
 * Fault Response Times [ms]
 ************************************************************************************************/
#define SHORT_CIRCUIT_RESPONSE_MS 1U   /**< HW trip reaction time */
#define OVERCURRENT_RESPONSE_LOOPS 2   /**< Delay before over-current trips */
#define OVERTEMP_RESPONSE_MS 500U      /**< Delay before over-temp trips */
#define UNDERVOLTAGE_RESPONSE_MS 1000U /**< Delay before under-voltage trips */
#define OVERVOLTAGE_RESPONSE_MS 1000U  /**< Delay before over-voltage trips */

/** @} */
