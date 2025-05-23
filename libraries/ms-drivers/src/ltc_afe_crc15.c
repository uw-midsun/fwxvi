/************************************************************************************************
 * @file   ltc_afe_crc15.c
 *
 * @brief  Source file to implement CRC15 for the LTC8611 AFE
 *
 * @date   2025-04-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "ltc_afe_crc15.h"

/* Intra-component Headers */

/**
 * @note x^{15} + x^{14} + x^{10} + x^{8} + x^{7} + x^{4} + x^{3} + x^{0}
 * @note Thus, divisor is: 0b1100010110011001 (0xC599)
 * @note 0xC599 - (2^15) == 0x4599 (Since crc15, 15th bit HAS to be 1)
 */

#define CRC_POLYNOMIAL 0x4599

static uint16_t s_crc15_table[256];

void crc15_init_table(void) {
  for (uint32_t i = 0; i < 256; ++i) {
    uint32_t remainder = i << 7;
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (remainder & 0x4000) {
        /* check MSB */
        remainder = (remainder << 1);
        remainder = (remainder ^ CRC_POLYNOMIAL);
      } else {
        remainder = (remainder << 1);
      }
    }

    s_crc15_table[i] = remainder & 0xFFFF;
  }
}

uint16_t crc15_calculate(uint8_t *data, size_t len) {
  /* CRC should be initialized to 16 (see datasheet p.53) */
  uint16_t remainder = 16;

  for (size_t i = 0; i < len; i++) {
    uint16_t addr = ((remainder >> 7) ^ data[i]) & 0xFF;
    remainder = (remainder << 8) ^ s_crc15_table[addr];
  }

  return remainder << 1;
}
