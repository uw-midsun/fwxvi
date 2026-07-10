/************************************************************************************************
 * @file    can_bl_entry.c
 *
 * @brief   Generic bl_port_* the CAN driver needs to drop a board into the bootloader
 *
 * @date    2026-06-04
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Intra-component Headers */
#include "bl_status.h"
#include "bootloader_port.h"
#include "can_hw.h"

/* The linker places these around the config page (last flash page), shared with the bootstrap
   and bootloader symbol contract, so the entry shim can read the config page identity */
extern uint32_t _config_start;
extern uint32_t _config_size;

static uint32_t __attribute__((section(".bl_noinit"))) s_boot_flag;

void bl_port_boot_flag_set(uint32_t value) {
  s_boot_flag = value;
}

uint32_t bl_port_boot_flag_get(void) {
  return s_boot_flag;
}

void bl_port_reset(void) {
  NVIC_SystemReset();
}

/* The app owns CAN through can_hw, it is already up by the time the announcer inits, so the
   transport's init becomes a no-op here instead of touching the peripheral */
BlStatus bl_port_can_init(uint32_t bitrate_kbps) {
  (void)bitrate_kbps;
  return BL_OK;
}

/* The driver feeds the shim with bl_entry_shim_feed_can, and the announcer only transmits, so
   neither reads CAN itself */
BlStatus bl_port_can_rx(uint32_t *id, uint8_t data[8], uint8_t *dlc) {
  (void)id;
  (void)data;
  (void)dlc;
  return BL_EMPTY;
}

/* HAL_GetTick is a plain volatile read, safe to call from the receive ISR where frames are fed */
uint32_t bl_port_now_ms(void) {
  return HAL_GetTick();
}

/* The app owns CAN through can_hw, the announcer paces its multi frame heartbeat from can_tx_board_info
   in task context, not the receive ISR, so use the blocking transmit which waits on a free mailbox,
   that is what lets the whole ANNOUNCE go out, a non blocking send would drop all but the first few
   frames once the three mailboxes fill */
BlStatus bl_port_can_tx(uint32_t id, const uint8_t *data, uint8_t dlc) {
  /* can_hw_transmit can spuriously return busy when its mailbox-free semaphore holds stale credit,
     the failed call drains that credit so a retry blocks for a real slot, this keeps a multi frame
     announce from truncating once the three mailboxes fill */
  for (uint8_t i = 0; i < 4U; i++) {
    if (can_hw_transmit(id, false, data, dlc) == STATUS_CODE_OK) {
      return BL_OK;
    }
  }
  return BL_ERR_INTERNAL;
}

/* Flash is memory mapped, so a config read is a plain copy from the mapped address */
BlStatus bl_port_flash_read(uint32_t addr, uint8_t *buf, uint32_t len) {
  if (buf == NULL) {
    return BL_ERR_INVALID_ARGS;
  }
  memcpy(buf, (const void *)(uintptr_t)addr, len);
  return BL_OK;
}

void bl_port_config_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = (uint32_t)(uintptr_t)&_config_start;
  }
  if (size != NULL) {
    *size = (uint32_t)(uintptr_t)&_config_size;
  }
}
