/************************************************************************************************
 * @file   test_ltc_afe.c
 *
 * @brief  Test file for ltc_afe_x86 driver
 *
 * @date   2025-01-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "adc.h"
#include "delay.h"
#include "log.h"
#include "tasks.h"
#include "test_helpers.h"
#include "unity.h"

/* Intra-component Headers */
#include "aux_sense.h"
#include "bms_carrier.h"
#include "bms_hw_defs.h"