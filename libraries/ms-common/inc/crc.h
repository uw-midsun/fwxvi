#pragma once

/************************************************************************************************
 * crc.h
 *
 * CRC Library Header file
 *
 * Created: 2024-11-20
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "status.h"

/* Intra-component Headers */

StatusCode crc_init(CRC_HandleTypeDef *hcrc);

uint32_t HAL_CRC_Calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength);
