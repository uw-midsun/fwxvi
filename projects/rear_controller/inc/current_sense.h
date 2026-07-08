#pragma once

/************************************************************************************************
 * @file   curretn_sense.h
 *
 * @brief Current sensing for Rear_Controller
 *
 * @date   2025-09-17
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */
#include "current_acs37800.h"
#include "rear_controller.h"

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

#define IS_USING_CURRENT_SENSE_REV_3 1U

#if (IS_USING_CURRENT_SENSE_REV_3 != 0U)
typedef struct {
  uint8_t fsr;                /* FSR = Vref / Gain */
  uint8_t mux_config_shunt;   /* shunt inputs */
  uint8_t mux_config_hv;      /* HV_bus inputs*/
  float shunt_resistance_ohm; /* shunt resistance in ohms*/
  uint32_t resistance_R6_ohm; /* resistance of R6 in ohms*/
  uint32_t resistance_R7_ohm; /* resistance of R7 in ohms */
} CurrentSenseConfigs;

#endif

/**
 * @brief   Initializes the current sense sub-system
 * @param   storage Pointer to the rear controller storage
 * @return  STATUS_CODE_OK if initialized succesfully
 *          STATUS_CODE_INVALID_ARGS if invalid parameter is passed in
 */
StatusCode current_sense_init(RearControllerStorage *rear_controller_storage);

/**
 * @brief   Run a current sensing cycle to update pack voltage and pack current readings
 * @return  STATUS_CODE_OK if current sensed successfully
 *          STATUS_CODE_UNINITIALIZED if not initialized
 */
StatusCode current_sense_run();

/** @} */
