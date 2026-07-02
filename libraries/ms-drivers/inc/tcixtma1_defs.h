#pragma once

/************************************************************************************************
 * @file    tcixtma1_defs.h
 *
 * @brief   Hydrogen Sensor TCIXTMA1 Defs
 *
 * @date    2026-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup tcixtma1_defs
 * @brief    tcixtma1_defs Firmware
 * @{
 */

#define TCIXTMA1_CONC_SCALE_FACTOR 0.01f

/* Trigger concentration measurement command (Datasheet Section 10.2) */
#define TCIXTMA1_TRIGGER_CONC_CMD 0xA8
#define TCIXTMA1_TRIGGER_CONC_CONFIG 0x01  // Only temperature compensation
#define TCIXTMA1_DEFAULT_RH 0x00
#define TCIXTMA1_DEFAULT_TEMP 0x7F
#define TCIXTMA1_DEFAULT_P 100

/* Defined processing time (Datasheet Section 7) */
#define TCIXTMA1_CONC_TIME 30

/** @} */
