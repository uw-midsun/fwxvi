#pragma once

/************************************************************************************************
 * @file   midFS_hal.h
 *
 * @brief  Header file for midFS_hal, which is used to communicate between file system and HAL
 *
 * @date   2025-09-12
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "midFS_types.h"

/* Intra-component Headers */
#include "status.h"
#include "stm32l433xx.h"
#include "stm32l4xx_hal_flash.h"

/**
 * @defgroup midFS_hal
 * @brief    midFS_hal Firmware
 * @{
 */

/**
 * Reads memory from HAL of size buffer_len at a specific address to an array buffer
 *
 * @return FS_STATUS_INVALID_ARGS if buffer is null
 */
FsStatus fs_hal_read(uint32_t address, uint8_t *buffer, size_t buffer_len);

/**
 * Writes memory from array buffer of size buffer_len to a specific address in HAL
 *
 * @return FS_STATUS_INVALID_ARGS if buffer is null
 */
FsStatus fs_hal_write(uint32_t address, uint8_t *buffer, size_t buffer_len);
/** @} */
