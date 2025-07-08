/************************************************************************************************
 * @file    fota_jump_handler.c
 *
 * @brief   FOTA jump handler
 *
 * @date    2025-06-14
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#ifndef MS_PLATFORM_X86
#include "cmsis_gcc.h"
#endif

/* Intra-component Headers */
#include "fota_jump_handler.h"
#include "fota_memory_map.h"

typedef void (*ResetHandler)(void);

FotaError fota_jump(FotaJumpRequest jump_request) {
  uint32_t *app_base = NULL;

  switch (jump_request) {
    case FOTA_JUMP_APPLICATION:
      app_base = (uint32_t *)APPLICATION_START_ADDRESS;
      break;
    case FOTA_JUMP_BOOTLOADER:
      app_base = (uint32_t *)BOOTLOADER_START_ADDRESS;
      break;
    case FOTA_JUMP_MIDSUN_BIOS:
      // TODO: assign appropriate address if applicable
      return FOTA_ERROR_JUMP_FAILED;
    default:
      return FOTA_ERROR_JUMP_FAILED;
  }

  if (app_base == NULL) {
    return FOTA_ERROR_JUMP_FAILED;
  }

#ifndef MS_PLATFORM_X86
  /* Disable interrupts before jumping */
  __disable_irq();

  uint32_t msp_value = app_base[0U];
  __set_MSP(msp_value);
#endif

  /* Get reset handler address (second word) */
  uintptr_t reset_vector_addr = (uintptr_t)app_base[1U];
  ResetHandler app_entry = (ResetHandler)reset_vector_addr;

  app_entry();

  return FOTA_ERROR_JUMP_FAILED;
}
