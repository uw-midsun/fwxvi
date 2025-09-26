#pragma once

/************************************************************************************************
 * @file    power_sensor.h
 *
 * @brief   Power Sensor
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"

/* Intra-component Headers */
/**
 * @defgroup power_sensor
 * @brief    power_sensor Firmware
 * @{
 */

// Initalize the mux select gpio pins, as well as PA2 (MUX out) as an analog to digital converter (ADC)
void power_sense_init();

// Loop over pins 0-11 input pins with the mux select pins and sample all of them into an array which stores all the current readings for everything
// Have an array that stores every single current sensor reading that you do, its already indexed 0-11, so store headlight in index 0, etc
void power_sense_run();

/** @} */