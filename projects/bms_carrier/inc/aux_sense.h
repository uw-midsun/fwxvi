#pragma once

/************************************************************************************************
 * @file   aux_sense.h
 *
 * @brief  Header file for bms_carrier
 *
 * @date   2025-01-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "bms.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "gpio.h"
/* Intra-component Headers */

/**
 * @defgroup bms_carrier
 * @brief    bms_carrier Firmware
 * @{
 */


#define R2_OHMS 10000
#define R1_OHMS 47000

StatusCode aux_sense_init(BmsStorage *storage);

StatusCode aux_sense_run(void);
/** @} */
