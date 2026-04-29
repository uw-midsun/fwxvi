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

static void s_update_bps_fault_can_fields(void) {
  set_rear_controller_status_triggers_bps_fault(rear_controller_storage->bps_fault);
  set_rear_controller_status_triggers_cell_at_fault(rear_controller_storage->bps_fault_cell);
}

StatusCode bps_fault_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;
  rear_controller_storage->bps_fault_cell = 0U;

  persist_init(&persist_storage, LAST_PAGE, &(rear_controller_storage->bps_fault), sizeof(rear_controller_storage->bps_fault), false);

  return STATUS_CODE_OK;
}

StatusCode bps_fault_commit() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  s_update_bps_fault_can_fields();
  persist_commit(&persist_storage);

  return STATUS_CODE_OK;
}

StatusCode trigger_bps_fault(BpsFault fault) {
  return trigger_bps_fault_with_cell(fault, 0U);
}

StatusCode trigger_bps_fault_with_cell(BpsFault fault, uint8_t cell_at_fault) {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (fault >= NUM_BPS_FAULTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage->bps_fault |= (1U << fault);
  if (cell_at_fault != 0U) {
    rear_controller_storage->bps_fault_cell = cell_at_fault;
  }

  s_update_bps_fault_can_fields();
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_FAULT);

  return STATUS_CODE_OK;
}

StatusCode bps_fault_clear() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  rear_controller_storage->bps_fault = 0U;
  rear_controller_storage->bps_fault_cell = 0U;
  s_update_bps_fault_can_fields();

  return STATUS_CODE_OK;
}
