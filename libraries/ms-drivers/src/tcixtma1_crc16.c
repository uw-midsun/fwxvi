/************************************************************************************************
 * @file    tcixtma_crc16.c
 *
 * @brief   Source file to implement CRC-16/CCITT-FALSE for the TCIXTMA1
 *
 * @date    2026-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */

/* Inter-component Headers */
#include "tcixtma1_crc16.h"

/* Intra-component Headers */

/**
 * @note CCITT-FALSE standard: x^{16} + x^{12} + x^{5} + x^{0}
 * @note Thus, divisor is: 0b10001000000100001 -> 0x1021 (since bit 16 always alogns and cancels out during Modulo-2 division)
 */

#define TCIXTMA1_CRC_POLYNOMIAL 0x1021

static uint16_t s_crc16_table[256];

void tcixtma1_crc16_init_table(void) {
  for (uint32_t i = 0; i < 256; ++i) {
    uint32_t remainder = i << 8;
    for (uint8_t bit = 8; bit > 0; --bit) {
      /* check MSB */
      if (remainder & 0x8000) {
        remainder = (remainder << 1) ^ TCIXTMA1_CRC_POLYNOMIAL;
      } else {
        remainder = (remainder << 1);
      }
    }

    s_crc16_table[i] = remainder & 0xFFFF;
  }
}

uint16_t tcixtma1_crc16_calculate(uint8_t *data, size_t len) {
  /* CRC-16/CCITT-FALSE standard with initialization value = 0xFFFF */
  uint16_t remainder = 0xFFFF;

  for (size_t i = 0; i < len; i++) {
    uint16_t addr = ((remainder >> 8) ^ data[i]) & 0xFF;
    remainder = (remainder << 8) ^ s_crc16_table[addr];
  }

  return remainder;
}
