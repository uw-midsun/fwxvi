/************************************************************************************************
 * @file    bl_port_bootstrap.c
 *
 * @brief   Minimal bl_port_* for the bootstrap, only flash read, regions and the jump are real
 *
 * @date    2026-06-04
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "stm32l4xx.h"

/* Intra-component Headers */
#include "bl_status.h"
#include "bootloader_port.h"

extern uint32_t _bootloader_start;
extern uint32_t _bootloader_size;
extern uint32_t _config_start;
extern uint32_t _config_size;
extern uint32_t _sram_start;
extern uint32_t _sram_size;

void bl_port_bootloader_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = (uint32_t)(uintptr_t)&_bootloader_start;
  }
  if (size != NULL) {
    *size = (uint32_t)(uintptr_t)&_bootloader_size;
  }
}

void bl_port_config_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = (uint32_t)(uintptr_t)&_config_start;
  }
  if (size != NULL) {
    *size = (uint32_t)(uintptr_t)&_config_size;
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

BlStatus bl_port_flash_read(uint32_t addr, uint8_t *buf, uint32_t len) {
  if (buf == NULL) {
    return BL_ERR_INVALID_ARGS;
  }
  memcpy(buf, (const void *)(uintptr_t)addr, len);
  return BL_OK;
}

void bl_port_jump(uint32_t vector_table_addr) {
  uint32_t sp = *(volatile uint32_t *)vector_table_addr;
  uint32_t entry = *(volatile uint32_t *)(vector_table_addr + 4U);

  __disable_irq();
  SCB->VTOR = vector_table_addr;
  __set_MSP(sp);
  __DSB();
  __ISB();
  ((void (*)(void))entry)();
}

/* The bootstrap links common only and never writes flash or touches CAN/UART, these satisfy the
   contract for any unreferenced paths and are never called */
void bl_port_app_region(uint32_t *start, uint32_t *size) {
  if (start != NULL) {
    *start = 0U;
  }
  if (size != NULL) {
    *size = 0U;
  }
}
uint32_t bl_port_flash_page_size(void) {
  return 0U;
}
BlStatus bl_port_flash_erase(uint32_t addr, uint32_t len) {
  (void)addr;
  (void)len;
  return BL_ERR_FLASH_ERASE;
}
BlStatus bl_port_flash_write(uint32_t addr, const uint8_t *buf, uint32_t len) {
  (void)addr;
  (void)buf;
  (void)len;
  return BL_ERR_FLASH_WRITE;
}
