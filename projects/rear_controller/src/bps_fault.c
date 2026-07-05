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
  set_rear_controller_status_triggers_bps_fault(rear_controller_storage->bps_fault_record.fault_code);
  set_rear_controller_status_triggers_cell_at_fault(rear_controller_storage->bps_fault_cell);
  set_bps_fault_info_extra_info(rear_controller_storage->bps_fault_record.extra_info.raw);
}

StatusCode bps_fault_init(RearControllerStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  rear_controller_storage = storage;
  rear_controller_storage->bps_fault_cell = 0U;

  status_ok_or_return(
      persist_init(&persist_storage, LAST_PAGE, &(rear_controller_storage->bps_fault_record), sizeof(rear_controller_storage->bps_fault_record), false));

  /* If a fault was latched before power-down, broadcast it on the first medium cycle so the
   * front controller blinks the BPS light on startup until drive state is entered */
  if (rear_controller_storage->bps_fault_record.fault_code != 0U) {
    s_update_bps_fault_can_fields();
  }

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
  BpsFaultData no_data = { .raw = 0U };
  return trigger_bps_fault_with_data(fault, 0U, no_data);
}

StatusCode trigger_bps_fault_with_cell(BpsFault fault, uint8_t cell_at_fault) {
  BpsFaultData no_data = { .raw = 0U };
  return trigger_bps_fault_with_data(fault, cell_at_fault, no_data);
}

StatusCode trigger_bps_fault_with_data(BpsFault fault, uint8_t cell_at_fault, BpsFaultData data) {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  if (fault >= NUM_BPS_FAULTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Latch the detail snapshot of the first (root) fault; do not overwrite while a fault is active */
  if (rear_controller_storage->bps_fault_record.fault_code == 0U) {
    rear_controller_storage->bps_fault_record.extra_info = data;
  }

  rear_controller_storage->bps_fault_record.fault_code |= ((uint32_t)1U << fault);
  if (cell_at_fault != 0U) {
    rear_controller_storage->bps_fault_cell = cell_at_fault;
  }

  /* Mark this as a live fault so the state manager opens the relays
   * A fault restored from flash on boot never sets this, so it broadcasts the BPS light without opening relays */
  rear_controller_storage->bps_fault_live = true;

  /* Update CAN fields and persist the latched fault to flash so it survives a power cycle */
  bps_fault_commit();
  rear_controller_state_manager_step(REAR_CONTROLLER_EVENT_FAULT);

  return STATUS_CODE_OK;
}

StatusCode bps_fault_clear() {
  if (rear_controller_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }

  rear_controller_storage->bps_fault_record.fault_code = 0U;
  rear_controller_storage->bps_fault_record.extra_info.raw = 0U;
  rear_controller_storage->bps_fault_cell = 0U;
  rear_controller_storage->bps_fault_live = false;

  /* Persist the cleared state so a stale fault is not re-broadcast on the next boot */
  bps_fault_commit();

  return STATUS_CODE_OK;
}
