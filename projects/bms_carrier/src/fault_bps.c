/************************************************************************************************
 * @file   fault_bps.c
 *
 * @brief  Source code for Battery protection system faults
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "fault_bps.h"
#include "relays.h"

struct FaultBpsStorage fault_bps_storage = {
    .fault_bitset = 0U
};

static BmsStorage *bms_storage;

StatusCode fault_bps_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  bms_storage = storage;
  bms_storage->fault_bps_storage = &fault_bps_storage;
  return STATUS_CODE_OK;
}

StatusCode fault_bps_set(uint8_t fault_bitmask) {
  if (bms_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }
  bms_relay_fault();

  bms_storage->fault_bps_storage->fault_bitset |= (1U << fault_bitmask);

  if (bms_storage->fault_bps_storage->fault_bitset & 0x7U) {
    bms_storage->fault_bps_storage->fault_bitset |= (1U << 15U);
  } else {
    bms_storage->fault_bps_storage->fault_bitset |= (1U << 14U);
  }

  set_battery_status_fault(bms_storage->fault_bps_storage->fault_bitset);
  return STATUS_CODE_OK;
}

StatusCode fault_bps_clear(uint8_t fault_bitmask) {
  if (bms_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }
  bms_storage->fault_bps_storage->fault_bitset &= ~(1U << fault_bitmask);
  return STATUS_CODE_OK;
}

uint16_t fault_bps_get() {
  if (bms_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }
  return (bms_storage->fault_bps_storage->fault_bitset & 0x3FFFU);
}
