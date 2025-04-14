#pragma once

/************************************************************************************************
 * @file   ltc_afe.h
 *
 * @brief  Header file to implement CRC15 for the LTC8611 AFE
 *
 * @date   2025-04-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

void crc15_init_table(void);

uint16_t crc15_calculate(uint8_t *data, size_t len);
