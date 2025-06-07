/************************************************************************************************
 * @file    sd_card_spi.c
 *
 * @brief   Sd Card Spi
 *
 * @date    2025-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>
#include <string.h>

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"

/* Intra-component Headers */
#include "sd_card_spi.h"

/* Standard Header*/

#define SD_TOKEN_START_BLOCK 0xFE
#define SD_TOKEN_MULTI_WRITE 0xFC
#define SD_TOKEN_STOP_TRAN 0xFD

#define R1_IDLE 0x01
#define R1_ILLEGAL 0x04

#define DATA_ACCEPTED 0x05

#define CRC_CMD0 0x95 /* valid CRC for CMD0 arg = 0   */
#define CRC_CMD8 0x87 /* valid CRC for CMD8 arg = 0x1AA */

#define T_INIT_MS 1000U
#define T_RW_MS 250U

#define CMD(n) (0x40 | (n))
enum {
  CMD0_GO_IDLE = 0,
  CMD8_SEND_IF_COND = 8,
  CMD13_SEND_STATUS = 13,
  CMD16_SET_BLKLEN = 16,
  CMD17_READ_SINGLE = 17,
  CMD24_WRITE_SINGLE = 24,
  CMD25_WRITE_MULTI = 25,
  CMD55_APP_CMD = 55,
  CMD58_READ_OCR = 58,
  ACMD23_PRE_ERASE = 23,
  ACMD41_SD_OP_COND = 41
};

/*  Local Helpers
--------------------------------------------------------------------------*/

StatusCode sd_card_init(SdSpiPort p, SdSpiSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode sd_read_blocks(SdSpiPort p, uint8_t *dst, uint32_t lba, uint32_t number_of_blocks) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode sd_write_blocks(SdSpiPort p, uint8_t *src, uint32_t lba, uint32_t number_of_blocks) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode sd_is_initialized(SdSpiPort p) {
  return STATUS_CODE_UNIMPLEMENTED;
}
