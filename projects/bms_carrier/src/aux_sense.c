/************************************************************************************************
 * @file   aux_sense.c
 *
 * @brief  Source code for Aux Battery sense
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "adc.h"
#include "gpio.h"

/* Intra-component Headers */
#include "aux_sense.h"
#include "bms_carrier_getters.h"
#include "bms_carrier_setters.h"
#include "bms_hw_defs.h"

/** @brief  Voltage divider R2 */
#define R2_OHMS 10000U
/** @brief  Voltage divider R1 */
#define R1_OHMS 47000U

struct AuxSenseStorage aux_sense_storage = { .batt_voltage_mv = 0U, .sense_adc = BMS_AUX_BATT_VOLTAGE_ADC };

static BmsStorage *bms_storage;

StatusCode aux_sense_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  bms_storage = storage;
  bms_storage->aux_sense_storage = &aux_sense_storage;
  gpio_init_pin(&bms_storage->aux_sense_storage->sense_adc, GPIO_ANALOG, GPIO_STATE_LOW);

  adc_add_channel(&bms_storage->aux_sense_storage->sense_adc);

  adc_init();

  return STATUS_CODE_OK;
}

StatusCode aux_sense_run() {
  adc_run();

  adc_read_converted(&bms_storage->aux_sense_storage->sense_adc, &bms_storage->aux_sense_storage->batt_voltage_mv);

  /* Reverse voltage divider calculation */
  bms_storage->aux_sense_storage->batt_voltage_mv = (bms_storage->aux_sense_storage->batt_voltage_mv) * (R2_OHMS + R1_OHMS) / (R2_OHMS);

  set_battery_status_aux_batt_v(bms_storage->aux_sense_storage->batt_voltage_mv);

  return STATUS_CODE_OK;
}
