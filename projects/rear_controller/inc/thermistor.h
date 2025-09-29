#pragma once

/************************************************************************************************
 * @file   thermistor.h
 *
 * @brief  Header file for thermistor module
 *
 * @date   2025-09-29
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Rear_Controller
 * @brief    Rear Controller Board Firmware
 * @{
 */

typedef struct {
  float resistance;
} ThermistorDataPoint;

extern const ThermistorDataPoint thermistor_table[];

#define THERMISTOR_TABLE_SIZE 102

uint16_t calculate_board_thermistor_temperature(uint16_t thermistor_voltage_mv);

/** @} */
