/************************************************************************************************
 * @file   adc.c
 *
 * @brief  ADC Library Source code
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "adc.h"

StatusCode adc_init(void) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_add_channel(GpioAddress *address) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_run(void) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_read_raw(GpioAddress *address, uint16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_read_converted(GpioAddress *address, uint16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}

void adc_deinit(void) {}

StatusCode adc_set_reading(GpioAddress *address, uint16_t reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}

