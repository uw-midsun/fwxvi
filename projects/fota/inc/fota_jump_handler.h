#pragma once

/************************************************************************************************
 * @file    fota_jump_handler.h
 *
 * @brief   FOTA Jump handler
 *
 * @date    2025-06-14
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "fota_error.h"
#include "fota_timeout.h"

/* Intra-component Headers */

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

typedef enum {
  FOTA_JUMP_APPLICATION, /**< Jump to application memory */
  FOTA_JUMP_BOOTLOADER,  /**< Jump to bootloader memory */
  FOTA_JUMP_MIDSUN_BIOS  /**< Jump to Midnight Sun BIOS */
} FotaJumpRequest;

FotaError fota_jump(FotaJumpRequest jump_request);

/** @} */
