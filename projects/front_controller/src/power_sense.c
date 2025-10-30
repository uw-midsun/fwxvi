/************************************************************************************************
 * @file    power_sensor.c
 *
 * @brief   Power Sensor
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "power_sense.h"

// Defines for MUX select pins
static const GpioAddress MUX_SEL_0 = { .port = GPIO_PORT_B, .pin = 12 };
static const GpioAddress MUX_SEL_1 = { .port = GPIO_PORT_B, .pin = 13 };
static const GpioAddress MUX_SEL_2 = { .port = GPIO_PORT_B, .pin = 14 };
static const GpioAddress MUX_SEL_3 = { .port = GPIO_PORT_B, .pin = 15 };
static const GpioAddress MUX_OUT = { .port = GPIO_PORT_A, .pin = 2 };

uint16_t power_sense_lo_current_calc(uint16_t sampled_voltage);
uint16_t power_sense_high_current_calc(uint16_t sampled_voltage);  // discord photo sent earlier measuring input current into pcb

// Initalize the mux select gpio pins, as well as PA2 (MUX out) as an analog to digital converter (ADC)
void power_sense_init() {
  gpio_init_pin(&MUX_SEL_0, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&MUX_SEL_1, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&MUX_SEL_2, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&MUX_SEL_3, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  // Initialize mux out as ADC pin
  gpio_init_pin(&MUX_OUT, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_init();
}

// Loop over pins 0-11 input pins with the mux select pins and sample all of them into an array which stores all the current readings for everything
// Have an array that stores every single current sensor reading that you do, its already indexed 0-11, so store headlight in index 0, etc
void power_sense_run();