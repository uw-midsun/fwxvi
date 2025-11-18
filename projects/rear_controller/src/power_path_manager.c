/************************************************************************************************
 * @file    power_path_manager.c
 *
 * @brief   Power Path Manager Source File
 *
 * @date    2025-05-28
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"

/* Intra-component Headers */
#include "power_path_manager.h"

static RearControllerStorage *rear_controller_storage = NULL;

// init gpio addresses
static GpioAddress pcs_vsense_gpio = REAR_CONTROLLER_PCS_VSENSE;
static GpioAddress pcs_isense_gpio = REAR_CONTROLLER_PCS_ISENSE;
static GpioAddress aux_vsense_gpio = REAR_CONTROLLER_AUX_VSENSE;
static GpioAddress aux_isense_gpio = REAR_CONTROLLER_AUX_ISENSE;
static GpioAddress pcs_valid1 = REAR_CONTROLLER_PCS_VALID1;
static GpioAddress pcs_valid2 = REAR_CONTROLLER_PCS_VALID2;
static GpioAddress aux_valid1 = REAR_CONTROLLER_AUX_VALID1;
static GpioAddress aux_valid2 = REAR_CONTROLLER_AUX_VALID2;

static StatusCode power_path_voltage_sense(GpioAddress *voltage_address, uint32_t *vin_mV);
static StatusCode power_path_current_sense(GpioAddress *current_address, int32_t *current_mA);

StatusCode power_path_manager_init(RearControllerStorage *storage) {

  if (storage == NULL){
    return STATUS_CODE_INVALID_ARGS;
  }
  rear_controller_storage = storage;
  

  // init pins
  gpio_init_pin(&pcs_vsense_gpio, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&pcs_isense_gpio, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&aux_vsense_gpio, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&aux_isense_gpio, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&pcs_valid1, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH);
  gpio_init_pin(&pcs_valid2, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH);
  gpio_init_pin(&aux_valid1, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH);
  gpio_init_pin(&aux_valid2, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH);

  // add adc channels
  adc_add_channel(&pcs_vsense_gpio);
  adc_add_channel(&pcs_isense_gpio);
  adc_add_channel(&aux_vsense_gpio);
  adc_add_channel(&aux_isense_gpio);

  return STATUS_CODE_OK;
}

StatusCode power_path_manager_run(void) {
  uint32_t pcs_voltage_mV = 0;
  int32_t pcs_current_mA = 0;
  uint32_t aux_voltage_mV = 0;
  int32_t aux_current_mA = 0;

  power_path_voltage_sense(&pcs_vsense_gpio, &pcs_voltage_mV);
  power_path_current_sense(&pcs_isense_gpio, &pcs_current_mA);

  power_path_voltage_sense(&aux_vsense_gpio, &aux_voltage_mV);
  power_path_current_sense(&aux_isense_gpio, &aux_current_mA);

  bool pcs_valid_1 = check_valid(pcs_valid1);
  bool pcs_valid_2 = check_valid(pcs_valid2);
  bool aux_valid_1 = check_valid(aux_valid1);
  bool aux_valid_2 = check_valid(aux_valid2);

  rear_controller_storage->pcs_voltage = pcs_voltage_mV;
  rear_controller_storage->pcs_current = pcs_current_mA;
  rear_controller_storage->aux_voltage = aux_voltage_mV;
  rear_controller_storage->aux_current = aux_current_mA;

  rear_controller_storage->pcs_valid = pcs_valid_1 && pcs_valid_2;
  rear_controller_storage->aux_valid = aux_valid_1 && aux_valid_2;

  return STATUS_CODE_OK;
}

// V sense
static StatusCode power_path_voltage_sense(GpioAddress *voltage_address, uint32_t *vin_mV) {
  uint16_t v_read_mV = 0;

  StatusCode status = adc_read_converted(voltage_address, &v_read_mV);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  // Voltage division
  uint32_t numerator = (uint32_t)v_read_mV * (R1 + R2);
  *vin_mV = (uint32_t)numerator / R2;

  return STATUS_CODE_OK;
}

// I sense
static StatusCode power_path_current_sense(GpioAddress *current_address, int32_t *current_mA) {
  uint16_t v_read_mV = 0;

  StatusCode status = adc_read_converted(current_address, &v_read_mV);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  // subtract offset
  int32_t delta_mV = (int32_t)v_read_mV - (int32_t)ZERO_OFFSET_MV;

  uint32_t numerator = delta_mV * 1000LL;
  *current_mA = (numerator / SENSITIVITY_MV_PER_A);

  return STATUS_CODE_OK;
}

// Check valid pins
bool check_valid(GpioAddress pin_address) {
  GpioState pin_state = gpio_get_state(&pin_address);

  if (pin_state == GPIO_STATE_LOW) {
    return true;
  } else {
    return false;
  }
}
