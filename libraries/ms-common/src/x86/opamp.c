/************************************************************************************************
 * @file   opamp.c
 *
 * @brief  OPAMP Library Source code
 *
 * @date   2025-01-30
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* Intra-component Headers */
#include "opamp.h"

typedef struct OpampState {
  bool configured;
  bool running;
  OpampConfig config;
  SemaphoreHandle_t mutex_handle;
  StaticSemaphore_t mutex_buffer;
} OpampState;

typedef struct OpampStatus {
  bool initialized;
  OpampState instances[NUM_OPAMP_INSTANCES];
} OpampStatus;

/* OPAMP peripheral status */
static OpampStatus s_opamp_status = { 0 };

StatusCode opamp_init(void) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode opamp_configure(OpampInstance instance, OpampConfig *config) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode opamp_start(OpampInstance instance) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode opamp_stop(OpampInstance instance) {
  return STATUS_CODE_UNIMPLEMENTED;
}
