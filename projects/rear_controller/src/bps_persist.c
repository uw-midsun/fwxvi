/************************************************************************************************
 * @file    bps_persist.c
 *
 * @brief   Bps Persist
 *
 * @date    2025-09-17
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "persist.h"
#include "rear_controller.h"
#define LAST_PAGE (NUM_FLASH_PAGES - 1)

PersistStorage persist_storage;
RearControllerStorage *rear_controller_storage;

StatusCode bps_persist_init(RearControllerStorage *storage) {
  rear_controller_storage = storage;
  persist_init(&persist_storage, LAST_PAGE, &(storage->bps_fault), sizeof(storage->bps_fault), false);

  return STATUS_CODE_OK;
}

StatusCode bps_commit() {
  persist_commit(&persist_storage);
  return STATUS_CODE_OK;
}
