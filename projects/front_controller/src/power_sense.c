/************************************************************************************************
 * @file    power_sense.c
 *
 * @brief   Power Sense
 *
 * @date    2025-07-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "power_sense.h"

#include "adc.h"
#include "front_controller_hw_defs.h"
#include "i2c.h"
#include "power_control_manager.h"
#include "power_control_outputs.h"

// Current sense ratio
#define kcs 1000

// Defines for MUX select pins
static const GpioAddress MUX_SEL_0 = FRONT_CONTROLLER_MUX_SEL_0;
static const GpioAddress MUX_SEL_1 = FRONT_CONTROLLER_MUX_SEL_1;
static const GpioAddress MUX_SEL_2 = FRONT_CONTROLLER_MUX_SEL_2;
static const GpioAddress MUX_SEL_3 = FRONT_CONTROLLER_MUX_SEL_3;
static const GpioAddress MUX_OUT = FRONT_CONTROLLER_MUX_OUTPUT;

// struct to store all the current readings could be an array
// use the original array to loop over mux select pins

// Helper functions to do the calculations
uint16_t power_sense_lo_current_calc(uint16_t sampledvoltage) {
  uint16_t lo_resistance = 5600;
  sampled_voltage = (sampled_voltage / lo_resistance) * kcs;
  return sampled_voltage;
}

uint16_t power_sense_high_current_calc(uint16_t sampledsvoltage) {
  uint16_t hi_resistance = 1210;
  sampled_voltage = (sampled_voltage / hi_resistance) * kcs;
  return sampled_voltage;
}

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

void power_sense_run(OutputGroup group) {
  OutputGroupDef *grp = output_group_map[group];

  if (grp == NULL) {
    return;
  }

  for (uint16_t i = 0; i < grp->num_outputs; i++) {
    int bit_mask_0 = 0001;
    int bit_mask_1 = 0010;
    int bit_mask_2 = 0100;
    int bit_mask_3 = 1000;

    if (i & bit_mask_0) {  // CHECK IF BIT 0 IS A 1
      gpio_set_state(&MUX_SEL_0, GPIO_STATE_HIGH);
    } else {
      gpio_set_state(&MUX_SEL_0, GPIO_STATE_LOW);
    }

    if (i & bit_mask_1) {
      gpio_set_state(&MUX_SEL_1, GPIO_STATE_HIGH);
    } else {
      gpio_set_state(&MUX_SEL_1, GPIO_STATE_LOW);
    }

    if (i & bit_mask_2) {
      gpio_set_state(&MUX_SEL_2, GPIO_STATE_HIGH);
    } else {
      gpio_set_state(&MUX_SEL_2, GPIO_STATE_LOW);
    }

    if (i & bit_mask_3) {
      gpio_set_state(&MUX_SEL_3, GPIO_STATE_HIGH);
    } else {
      gpio_set_state(&MUX_SEL_3, GPIO_STATE_LOW);
    }

    adc_run();
    StatusCode sc = adc_read_converted(&MUX_OUT, &sampled_voltage);

    current_low_readings[i] = power_sense_lo_current_calc(sampled_voltage);
    current_hi_readings[i] = power_sense_high_current_calc(sampled_voltage);
  }
}
