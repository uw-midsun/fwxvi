/************************************************************************************************
 * @file   crc.c
 *
 * @brief  CRC Library Source file
 *
 * @date   2025-02-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "crc.h"
#include "status.h"

StatusCode crc_init(CRCLength crc_length) {
  (void) crc_length;
  return STATUS_CODE_UNIMPLEMENTED;
}

uint32_t crc_calculate(const uint32_t *data, size_t length) {
  (void) data;
  (void) length;
  return STATUS_CODE_UNIMPLEMENTED;
}

void crc_reset(void) {
  return;
}
