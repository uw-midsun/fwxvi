/************************************************************************************************
 * @file    bps_fault.c
 *
 * @brief   BPS Fault source file
 *
 * @date    2025-09-17
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "persist.h"

/* Intra-component Headers */
#include "bps_fault.h"
#include "rear_controller.h"
#include "rear_controller_setters.h"
#include "rear_controller_state_manager.h"

#define LAST_PAGE (NUM_FLASH_PAGES - 1)

static PersistStorage persist_storage;
static RearControllerStorage *rear_controller_storage = NULL;

StatusCode bps_fault_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;

  persist_init(&persist_storage, LAST_PAGE, &(rear_controller_storage->bps_fault), sizeof(rear_controller_storage->bps_fault), false);

  return STATUS_CODE_OK;
}

StatusCode bps_fault_commit() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  set_rear_controller_status_bps_fault(rear_controller_storage->bps_fault);
  persist_commit(&persist_storage);

  return STATUS_CODE_OK;
}

StatusCode trigger_bps_fault(BpsFault fault) {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (fault >= NUM_BPS_FAULTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage->bps_fault |= (1U << fault);
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_FAULT);

  return STATUS_CODE_OK;
}

StatusCode bps_fault_clear() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  rear_controller_storage->bps_fault = 0U;

  return STATUS_CODE_OK;
}
