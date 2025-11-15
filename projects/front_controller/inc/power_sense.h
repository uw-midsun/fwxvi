#pragma once

/************************************************************************************************
 * @file    power_sense.h
 *
 * @brief   Power Sense
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"
#include "power_control_outputs.h"

/* Intra-component Headers */
/**
 * @defgroup power_sensor
 * @brief    power_sensor Firmware
 * @{
 */

#define NUM_SENSE_CH 12
// Initalize the mux select gpio pins, as well as PA2 (MUX out) as an analog to digital converter (ADC)
void power_sense_init();

// Loop over pins 0-11 input pins with the mux select pins and sample all of them into an array which stores all the current readings for everything
// Have an array that stores every single current sensor reading that you do, its already indexed 0-11, so store headlight in index 0, etc
void power_sense_run(OutputGroup group);

// Helper functions to do the calculations
uint16_t power_sense_lo_cuarrent_calc(uint16_t sampledvoltage);
uint16_t power_sense_high_current_calc(uint16_t sampledvoltage);  // discord photo sent earlier measuring input current into pcb

uint8_t channel;
uint16_t sampled_voltage;

uint16_t current_low_readings[NUM_SENSE_CH];
uint16_t current_hi_readings[NUM_SENSE_CH];

/** @} */
