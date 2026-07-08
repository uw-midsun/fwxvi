/************************************************************************************************
 * @file    bl_port_stm32l4.c
 *
 * @brief   STM32L4 bl_port_* implementation, wires the chip agnostic bootloader to this repo
 *
 * @date    2026-06-03
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <string.h>

/* Inter-component Headers */
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

/* Intra-component Headers */
#include "bl_status.h"
#include "bootloader_port.h"
#include "bootloader_user_config.h"

/* Linker provided geometry, the single source of truth (generated from bootloader_user_config.h) */
extern uint32_t _bootloader_start;
extern uint32_t _bootloader_size;
extern uint32_t _app_start;
extern uint32_t _app_size;
extern uint32_t _config_start;
extern uint32_t _config_size;
extern uint32_t _flash_page_size;
extern uint32_t _sram_start;
extern uint32_t _sram_size;

/* No-init RAM word for the app entry shim, survives a warm reset (see sections_common.ld) */
static uint32_t __attribute__((section(".bl_noinit"))) s_boot_flag;

static CAN_HandleTypeDef s_can;

typedef struct {
  uint32_t prescaler;
  uint32_t bs1;
  uint32_t bs2;
} CanTiming;

static CanTiming can_timing(uint32_t bitrate_kbps) {
  switch (bitrate_kbps) {
    case 125U: return (CanTiming){ 40U, CAN_BS1_13TQ, CAN_BS2_2TQ };
    case 250U: return (CanTiming){ 20U, CAN_BS1_13TQ, CAN_BS2_2TQ };
    case 1000U: return (CanTiming){ 5U, CAN_BS1_13TQ, CAN_BS2_2TQ };
    case 500U:
    default: return (CanTiming){ 10U, CAN_BS1_11TQ, CAN_BS2_4TQ };
  }
}

BlStatus bl_port_can_init(uint32_t bitrate_kbps) {
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef gpio = {
    .Pin = GPIO_PIN_8 | GPIO_PIN_9,
    .Mode = GPIO_MODE_AF_PP,
    .Pull = GPIO_PULLUP,
    .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
    .Alternate = GPIO_AF9_CAN1,
  };
  HAL_GPIO_Init(GPIOB, &gpio);

  __HAL_RCC_CAN1_CLK_ENABLE();
  __HAL_RCC_CAN1_FORCE_RESET();
  __HAL_RCC_CAN1_RELEASE_RESET();

  CanTiming t = can_timing(bitrate_kbps);
  s_can.Instance = CAN1;
  s_can.Init.Prescaler = t.prescaler;
  s_can.Init.Mode = CAN_MODE_NORMAL;
  s_can.Init.SyncJumpWidth = CAN_SJW_1TQ;
  s_can.Init.TimeSeg1 = t.bs1;
  s_can.Init.TimeSeg2 = t.bs2;
  s_can.Init.TimeTriggeredMode = DISABLE;
  s_can.Init.AutoBusOff = ENABLE;
  s_can.Init.AutoWakeUp = DISABLE;
  s_can.Init.AutoRetransmission = DISABLE;
  s_can.Init.ReceiveFifoLocked = DISABLE;
  s_can.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&s_can) != HAL_OK) {
    return BL_ERR_CAN;
  }

  /* Accept all into FIFO0, the transport software filters by role id (bl_transport_can) */
  CAN_FilterTypeDef filter = {
    .FilterBank = 0,
    .FilterMode = CAN_FILTERMODE_IDMASK,
    .FilterScale = CAN_FILTERSCALE_32BIT,
    .FilterIdHigh = 0x0000,
    .FilterIdLow = 0x0000,
    .FilterMaskIdHigh = 0x0000,
    .FilterMaskIdLow = 0x0000,
    .FilterFIFOAssignment = CAN_RX_FIFO0,
    .FilterActivation = ENABLE,
    .SlaveStartFilterBank = 14,
  };
  if (HAL_CAN_ConfigFilter(&s_can, &filter) != HAL_OK) {
    return BL_ERR_CAN;
  }
  if (HAL_CAN_Start(&s_can) != HAL_OK) {
    return BL_ERR_CAN;
  }
  return BL_OK;
}

BlStatus bl_port_can_rx(uint32_t *id, uint8_t data[8], uint8_t *dlc) {
  if (HAL_CAN_GetRxFifoFillLevel(&s_can, CAN_RX_FIFO0) == 0U) {
    return BL_EMPTY;
  }
  CAN_RxHeaderTypeDef header;
  uint8_t buf[8] = { 0 };
  if (HAL_CAN_GetRxMessage(&s_can, CAN_RX_FIFO0, &header, buf) != HAL_OK) {
    return BL_ERR_CAN;
  }
  *id = (header.IDE == CAN_ID_EXT) ? header.ExtId : header.StdId;
  *dlc = (uint8_t)header.DLC;
  memcpy(data, buf, 8U);
  return BL_OK;
}

BlStatus bl_port_can_tx(uint32_t id, const uint8_t *data, uint8_t dlc) {
  if ((data == NULL) || (dlc > 8U)) {
    return BL_ERR_INVALID_ARGS;
  }
  CAN_TxHeaderTypeDef header = {
    .StdId = id,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = dlc,
    .TransmitGlobalTime = DISABLE,
  };
  uint32_t deadline = HAL_GetTick() + 20U;
  while (HAL_CAN_GetTxMailboxesFreeLevel(&s_can) == 0U) {
    if (HAL_GetTick() >= deadline) {
      return BL_ERR_CAN;
    }
  }
  uint32_t mailbox = 0U;
  if (HAL_CAN_AddTxMessage(&s_can, &header, (uint8_t *)data, &mailbox) != HAL_OK) {
    return BL_ERR_CAN;
  }
  return BL_OK;
}

/* A leaf CAN bootloader has no upstream, the UART transport is still linked so provide stubs */
BlStatus bl_port_uart_init(uint32_t baud) {
  (void)baud;
  return BL_OK;
}

BlStatus bl_port_uart_rx(uint8_t *buf, uint32_t *len) {
  (void)buf;
  if (len != NULL) {
    *len = 0U;
  }
  return BL_OK;
}

BlStatus bl_port_uart_tx(const uint8_t *buf, uint32_t len) {
  (void)buf;
  (void)len;
  return BL_OK;
}

uint32_t bl_port_flash_page_size(void) {
  return (uint32_t)(uintptr_t)&_flash_page_size;
}

void bl_port_config_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = (uint32_t)(uintptr_t)&_config_start;
  }
  if (size != NULL) {
    *size = (uint32_t)(uintptr_t)&_config_size;
  }
}

void bl_port_app_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = (uint32_t)(uintptr_t)&_app_start;
  }
  if (size != NULL) {
    *size = (uint32_t)(uintptr_t)&_app_size;
  }
}

void bl_port_bootloader_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = (uint32_t)(uintptr_t)&_bootloader_start;
  }
  if (size != NULL) {
    *size = (uint32_t)(uintptr_t)&_bootloader_size;
  }
}

void bl_port_sram_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = (uint32_t)(uintptr_t)&_sram_start;
  }
  if (size != NULL) {
    *size = (uint32_t)(uintptr_t)&_sram_size;
  }
}

BlStatus bl_port_flash_erase(uint32_t addr, uint32_t len) {
  uint32_t page_size = bl_port_flash_page_size();
  uint32_t first = (addr - FLASH_BASE) / page_size;
  uint32_t n = (len + page_size - 1U) / page_size;

  FLASH_EraseInitTypeDef erase = {
    .TypeErase = FLASH_TYPEERASE_PAGES,
    .Banks = FLASH_BANK_1,
    .Page = first,
    .NbPages = n,
  };
  uint32_t page_error = 0U;
  if (HAL_FLASH_Unlock() != HAL_OK) {
    return BL_ERR_FLASH_ERASE;
  }
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &page_error);
  HAL_FLASH_Lock();
  return (status == HAL_OK) ? BL_OK : BL_ERR_FLASH_ERASE;
}

BlStatus bl_port_flash_write(uint32_t addr, const uint8_t *buf, uint32_t len) {
  if ((buf == NULL) || ((len % 8U) != 0U) || ((addr % 8U) != 0U)) {
    return BL_ERR_NOT_ALIGNED;
  }
  if (HAL_FLASH_Unlock() != HAL_OK) {
    return BL_ERR_FLASH_WRITE;
  }
  for (uint32_t off = 0U; off < len; off += 8U) {
    uint64_t word;
    memcpy(&word, &buf[off], 8U);
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + off, word) != HAL_OK) {
      HAL_FLASH_Lock();
      return BL_ERR_FLASH_WRITE;
    }
  }
  HAL_FLASH_Lock();
  /* HAL_FLASH_Program does not invalidate the D-cache, reset it so reads see the new flash */
  __HAL_FLASH_DATA_CACHE_DISABLE();
  __HAL_FLASH_DATA_CACHE_RESET();
  __HAL_FLASH_DATA_CACHE_ENABLE();
  return BL_OK;
}

BlStatus bl_port_flash_read(uint32_t addr, uint8_t *buf, uint32_t len) {
  if (buf == NULL) {
    return BL_ERR_INVALID_ARGS;
  }
  memcpy(buf, (const void *)(uintptr_t)addr, len);
  return BL_OK;
}

uint32_t bl_port_now_ms(void) {
  return HAL_GetTick();
}

void bl_port_jump(uint32_t vector_table_addr) {
  uint32_t sp = *(volatile uint32_t *)vector_table_addr;
  uint32_t entry = *(volatile uint32_t *)(vector_table_addr + 4U);

  /* Reset clocks to the reset default so the app's own clock setup starts clean (it cannot
     reconfigure the PLL while the PLL is the live system clock), then quiesce all interrupts */
  __disable_irq();
  HAL_RCC_DeInit();
  SysTick->CTRL = 0U;
  SysTick->VAL = 0U;
  for (uint32_t i = 0U; i < 8U; i++) {
    NVIC->ICER[i] = 0xFFFFFFFFU;
    NVIC->ICPR[i] = 0xFFFFFFFFU;
  }

  SCB->VTOR = vector_table_addr;
  __set_MSP(sp);
  __DSB();
  __ISB();

  /* The app expects the reset default of interrupts enabled */
  __enable_irq();
  ((void (*)(void))entry)();
}

void bl_port_reset(void) {
  NVIC_SystemReset();
}

uint32_t bl_port_boot_flag_get(void) {
  return s_boot_flag;
}

void bl_port_boot_flag_set(uint32_t value) {
  s_boot_flag = value;
}
