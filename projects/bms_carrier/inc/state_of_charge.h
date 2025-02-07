#pragma once

/************************************************************************************************
 * @file   state_of_charge.h
 *
 * @brief  Header file for State of Charge
 *
 * @date   2025-02-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bms_carrier.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */

/** @brief  Internal resistance of cells from testing */
#define CELL_INTERNAL_RESISTANCE_mOHMS    22U
/** @brief  9 modules, each module is 8P4S */
#define PACK_INTERNAL_RESISTANCE_mOHMS    (CELL_INTERNAL_RESISTANCE_mOHMS * 9U * 4U) / 8U
/** @brief  Volts to millivolts conversion */
#define VOLTS_TO_mV                       1000U
/** @brief  Lookup table size */
#define LUT_SIZE                          41U

/**
 * @brief   State of charge storage
 */
struct StateOfChargeStorage {
  uint32_t last_time;       /**< Last measurement time */
  float i_soc;              /**< Current state of charge with Coulomb Counting */
  float v_soc;              /**< Voltage state of charge with voltage mapping */
  float averaged_soc;       /**< Averaged state of charge */
  int32_t last_current;     /**< Last current measurement */
};

/**
 * @brief   Initializes the state of charge
 * @param   storage Pointer to the BMS storage
 * @return  STATUS_CODE_OK if state of charge initialization succeeded
 *          STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode state_of_charge_init(BmsStorage *storage);


/**
 * @brief   Updates the state of charge value
 * @return  STATUS_CODE_OK if state of charge calculation succeeded
 */
StatusCode update_state_of_chrage();

/**
 * @brief   Coulomb counting state of charge
 * @details Uses trapezoidal rule for integrating the current over discrete-time
 */
void coulomb_counting_soc();

/**
 * @brief   Open circuit voltage state of charge
 * @details Uses lookup table to determine state of charge
 */
void ocv_voltage_soc();

/**
 * @brief   Perdicts the open-circuit voltage
 * @return  Float value accounting for pack's internal resistance
 */
float perdict_ocv_voltage();

#ifdef MS_TEST

/**
 * @brief   Ramps the voltage weight to account for non-linear regions of state of charge
 * @details During linear regions (80% - 20%) the current weighting is maximized
 *          During non-linear regions the voltage weighting is maximized
 */
void ramp_voltage_weight();

/**
 * @brief   Setter for the last time
 */
void set_last_time(uint32_t last_time);

/**
 * @brief   Setter for the current state of charge
 */
void set_i_soc(float i_soc);

/**
 * @brief   Setter for the voltage state of charge
 */
void set_v_soc(float v_soc);

/**
 * @brief   Setter for the averaged state of charge
 */
void set_averaged_soc(float averaged_soc);

/**
 * @brief   Setter for the last current reading
 */
void set_last_current(int32_t last_current);

/**
 * @brief   Getter for the last time
 */
uint32_t get_last_time(void);

/**
 * @brief   Getter for the current state of charge
 */
float get_i_soc(void);

/**
 * @brief   Getter for the voltage state of charge
 */
float get_v_soc(void);

/**
 * @brief   Getter for the averaged state of charge
 */
float get_averaged_soc(void);

/**
 * @brief   Getter for the last current reading
 */
int32_t get_last_current(void);

/**
 * @brief   Getter for the voltage weight
 */
float get_voltage_weight(void);

#endif

/** @} */
