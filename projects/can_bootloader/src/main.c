/************************************************************************************************
 * @file    main.c
 *
 * @brief   CAN bootloader entry, wires the ms-bootloader core to the STM32L4 port and runs it
 *
 * @date    2026-06-03
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Intra-component Headers */
#include "bl_transport_can.h"
#include "bootloader.h"
#include "bootloader_config.h"
#include "bootloader_user_config.h"
#include "mcu.h"

extern uint32_t _bootloader_start;

void SysTick_Handler(void) {
  HAL_IncTick();
}

/* node_id comes from the flash resident BootConfig (written by `scons image`), so this same
   binary is valid for every board on this chip, BL_NODE_ID is only the fallback before a chip
   has ever been imaged */
static uint16_t resolve_node_id(void) {
  BootConfig cfg;
  if (bl_config_read(&cfg) == BL_OK) {
    return cfg.board_id;
  }
  return BL_NODE_ID;
}

int main(void) {
  __enable_irq();
  SCB->VTOR = (uint32_t)(uintptr_t)&_bootloader_start;

  if (HAL_Init() != HAL_OK) {
    while (true) {
    }
  }

  /* CAN bit timing assumes an 80 MHz PCLK1, bring the PLL up before any peripheral init */
  if (mcu_clock_init() != STATUS_CODE_OK) {
    while (true) {
    }
  }

  const uint16_t node_id = resolve_node_id();

  const BlCanSettings can_settings = {
    .bitrate_kbps = BL_CAN_BITRATE_KBPS,
    .xfer_id_base = BL_XFER_ID_BASE,
    .enter_id = BL_ENTER_ID,
    .node_id = node_id,
  };

  const BlBootloaderConfig config = {
    .downstream = &bl_transport_can,
    .downstream_settings = &can_settings,
    .upstream = NULL,
    .upstream_settings = NULL,
    .node_id = node_id,
  };

  if (bl_bootloader_init(&config) != BL_OK) {
    while (true) {
    }
  }

  for (;;) {
    bl_bootloader_poll();
  }
  return 0;
}
