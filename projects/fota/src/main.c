/************************************************************************************************
 * @file    main.c
 *
 * @brief   FOTA gateway entry, the same ms-bootloader core as can_bootloader but with the
 *          XBee UART wired in as the upstream transport so this board relays a host firmware
 *          update from upstream (UART) onto the downstream bus (CAN)
 *
 * @date    2026-06-29
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Intra-component Headers */
#include "bl_transport_can.h"
#include "bl_transport_uart.h"
#include "bootloader.h"
#include "bootloader_config.h"
#include "bootloader_user_config.h"
#include "mcu.h"

/* TODO(fota): source the baud and the gateway selection from board.toml so the generated
   bootloader_user_config.h carries BL_UART_BAUD + BL_FEATURE_GATEWAY instead of these locals */
#define FOTA_UART_BAUD 230400U

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

static const BlUartSettings s_uart_settings = {
  .baud = FOTA_UART_BAUD,
};

int main(void) {
  __enable_irq();
  SCB->VTOR = (uint32_t)(uintptr_t)&_bootloader_start;

  if (HAL_Init() != HAL_OK) {
    while (true) {
    }
  }

  /* CAN and UART bit timing both assume an 80 MHz PCLK1, bring the PLL up first */
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

  /* Gateway build: upstream (XBee UART) present, so is_gateway() is true in the core and this
     node relays host datagrams addressed elsewhere onto the downstream CAN bus */
  const BlBootloaderConfig config = {
    .downstream = &bl_transport_can,
    .downstream_settings = &can_settings,
    .upstream = &bl_transport_uart,
    .upstream_settings = &s_uart_settings,
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
