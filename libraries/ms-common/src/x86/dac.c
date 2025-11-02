/************************************************************************************************
 * @file   dac.c
 *
 * @brief  DAC Library Source code
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* Intra-component Headers */
#include "dac.h"

typedef struct DacChannelState {
  bool enabled;
  uint16_t current_value;
  SemaphoreHandle_t mutex_handle;
  StaticSemaphore_t mutex_buffer;
} DacChannelState;

typedef struct DacStatus {
  bool initialized;
  DacChannelState channels[NUM_DAC_CHANNELS];
} DacStatus;

/* DAC peripheral status */
static DacStatus s_dac_status = { 0 };

StatusCode dac_init(void) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode dac_enable_channel(DacChannel channel) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode dac_set_raw(DacChannel channel, uint16_t value) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode dac_set_voltage(DacChannel channel, uint16_t voltage_mv) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode dac_get_raw(DacChannel channel, uint16_t *value) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode dac_get_voltage(DacChannel channel, uint16_t *voltage_mv) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode dac_disable_channel(DacChannel channel) {
  return STATUS_CODE_UNIMPLEMENTED;
}
