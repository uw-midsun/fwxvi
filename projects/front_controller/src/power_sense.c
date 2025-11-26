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
#include "delay.h"
#include "front_controller_hw_defs.h"
#include "i2c.h"
#include "log.h"
#include "power_control_manager.h"
#include "power_control_outputs.h"

// Current sense ratio
#define kcs 1000

// Defines for MUX select pins
static GpioAddress MUX_SEL_0 = FRONT_CONTROLLER_MUX_SEL_0;
static GpioAddress MUX_SEL_1 = FRONT_CONTROLLER_MUX_SEL_1;
static GpioAddress MUX_SEL_2 = FRONT_CONTROLLER_MUX_SEL_2;
static GpioAddress MUX_SEL_3 = FRONT_CONTROLLER_MUX_SEL_3;
static GpioAddress MUX_OUT = FRONT_CONTROLLER_MUX_OUTPUT;

// struct to store all the current readings could be an array
// use the original array to loop over mux select pins

// Helper functions to do the calculations
uint16_t power_sense_lo_current_calc(uint16_t sampled_voltage) {
  uint16_t lo_resistance = 5600;
  sampled_voltage = (uint16_t)(((float)sampled_voltage / (float)lo_resistance) * (float)kcs);
  return sampled_voltage;
}

uint16_t power_sense_high_current_calc(uint16_t sampled_voltage) {
  uint16_t hi_resistance = 1210;
  sampled_voltage = (uint16_t)(((float)sampled_voltage / (float)hi_resistance) * (float)kcs);
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
  adc_add_channel(&MUX_OUT);
  adc_init();
}

void power_sense_run(OutputGroup group) {
  OutputGroupDef *grp = output_group_map[group];

  if (grp == NULL) {
    return;
  }

  for (uint16_t i = 0; i < grp->num_outputs; i++) {
    int bit_mask_0 = 0b0001;
    int bit_mask_1 = 0b0010;
    int bit_mask_2 = 0b0100;
    int bit_mask_3 = 0b1000;

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

    uint16_t sampled_voltage;

    StatusCode sc = adc_read_converted(&MUX_OUT, &sampled_voltage);
    if (sc != STATUS_CODE_OK) {
      LOG_DEBUG("ADC read failed with exit code: %d", sc);
    }

    current_low_readings[i] = power_sense_lo_current_calc(sampled_voltage);
    current_hi_readings[i] = power_sense_high_current_calc(sampled_voltage);
    LOG_DEBUG("GROUP %d | ADC %d | HIGH %d | LOW %d\r\n", i, sampled_voltage, power_sense_high_current_calc(sampled_voltage), power_sense_lo_current_calc(sampled_voltage));
    delay_ms(10);
  }
}
