/************************************************************************************************
 * @file    fota_jump_handler.c
 *
 * @brief   FOTA jump handler
 *
 * @date    2025-06-14
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "fota_jump_handler.h"

#include "fota_memory_map.h"

/* Intra-component Headers */

FotaError fota_jump(FotaJumpRequest jump_request) {
  switch (jump_request) {
    case FOTA_JUMP_APPLICATION:
      __asm volatile(
          "LDR     R0, =_application_start  \n"
          "MSR     MSP, R0                  \n"
          "LDR     R1, [R0, #4]             \n"
          "BX      R1                       \n");
      break;
    case FOTA_JUMP_BOOTLOADER:
      /* Unsupported jump */
      break;
    case FOTA_JUMP_MIDSUN_BIOS:
      /* TODO */
      break;
    default:
      break;
  }
  return FOTA_ERROR_JUMP_FAILED;
}
