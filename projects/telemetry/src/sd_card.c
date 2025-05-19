

#include "sd_card.h"

#include <string.h>

#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"

/*--------------------------------------------------------------------------*/
/*  Constants & macros                                                      */

#define R1_IDLE 0x01

#define DATA_ACCEPTED 0x05

#define CRC_CMD0 0x95   /* valid CRC for CMD0 arg = 0   */
#define CRC_CMD8 0x87   /* valid CRC for CMD8 arg = 0x1AA */

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

/*--------------------------------------------------------------------------*/
/*  Local helpers                                                           */

static inline SPI_HandleTypeDef *prv_port_to_handle(SpiPort p) {
  return (p == SPI_PORT_1) ? &hspi1 : (p == SPI_PORT_2) ? &hspi2 : &hspi3;
}

/* May CHange, based on spi.h implementation */
static inline void prv_cs_low(SpiPort p) {
  if (p == SPI_PORT_1)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
  else if (p == SPI_PORT_2)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  else
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
}
static inline void prv_cs_high(SpiPort p) {
  if (p == SPI_PORT_1)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
  else if (p == SPI_PORT_2)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
}

/* Wrapper around HAL_SPI_TransmitReceive */
static HAL_StatusTypeDef prv_txrx(SpiPort p, uint8_t *tx, uint8_t *rx, uint16_t len) {
  return HAL_SPI_TransmitReceive(prv_port_to_handle(p), tx, rx, len, HAL_MAX_DELAY);
}

static uint8_t prv_transfer_byte(SpiPort p, uint8_t b) {
  uint8_t r;
  prv_txrx(p, &b, &r, 1);
  return r;
}

static uint32_t prv_wait_ready(SpiPort p, uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick();
  while (HAL_GetTick() - start < timeout_ms)
    if (prv_transfer_byte(p, 0xFF) == 0xFF) {
      return 0;
    }
  
  LOG_DEBUG("sd_card: wait ready timed out\n");
  return 1; /* timed-out */
}

static uint8_t prv_send_cmd(SpiPort p, uint8_t cmd, uint32_t arg, uint8_t crc) {
  uint8_t frame[6];

  /* Big Endian */
  frame[0] = CMD(cmd);
  frame[1] = arg >> 24;
  frame[2] = arg >> 16;
  frame[3] = arg >> 8;
  frame[4] = arg;
  frame[5] = crc;

  prv_cs_low(p);
  prv_wait_ready(p, T_RW_MS);

  uint8_t dummy;
  prv_txrx(p, frame, &dummy, 6);

  /* wait response (MSB clear) */
  for (uint8_t i = 0; i < 10; i++) {
    uint8_t r = prv_transfer_byte(p, 0xFF);
    if (!(r & 0x80)) return r;
  }

  LOG_DEBUG("sd_card: no response to CMD%d\n", cmd);
  return 0xFF; /* no valid response */
}

/*--------------------------------------------------------------------------*/
/*  Global state                                                             */

static bool sdhc = false;

/*--------------------------------------------------------------------------*/
/*  Public API                                                               */

StatusCode sd_card_init(SpiPort p) {
  /* 80 (8 * 10) dummy clocks with CS high to 'wake' up card */
  prv_cs_high(p);
  for (uint8_t i = 0; i < 10; i++) {
    prv_transfer_byte(p, 0xFF);
  }

  /* reset + enter SPI mode */
  if (prv_send_cmd(p, CMD0_GO_IDLE, 0, CRC_CMD0) != R1_IDLE) {
    prv_cs_high(p);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  
  /* voltage check */
  if (prv_send_cmd(p, CMD8_SEND_IF_COND, 0x1AA, CRC_CMD8) != R1_IDLE) {
    prv_cs_high(p);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  uint8_t r7[4];
  prv_txrx(p, r7, r7, 4); /* discard echo bytes */

  /* ACMD41 loop */
  uint32_t start = HAL_GetTick(); 
  do {
    prv_send_cmd(p, CMD55_APP_CMD, 0, 0xFF); /* Initiliazes in prep for ACMD command */
    if (prv_send_cmd(p, ACMD41_SD_OP_COND, 1UL << 30, 0xFF) == 0) 
      break;
  } while (HAL_GetTick() - start < T_INIT_MS);

  if (HAL_GetTick() - start >= T_INIT_MS) {
    prv_cs_high(p);
    LOG_DEBUG("sd_card: ACMD41 timed out\n");
    return STATUS_CODE_TIMEOUT;
  }

  /* Dk what type of card it is -> So read OCR to check CCS */
  if (prv_send_cmd(p, CMD58_READ_OCR, 0, 0xFF) != 0) { 
    prv_cs_high(p);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  uint8_t ocr[4];
  prv_txrx(p, ocr, ocr, 4);
  sdhc = (ocr[0] & 0x40); /* Bit 30 has CCS flag */

  prv_cs_high(p);

  /* switch SPI prescaler to fast (e.g. /4) here if desired */
  return STATUS_CODE_OK;
}

StatusCode sd_read_blocks(SpiPort p, uint8_t *dst, uint32_t lba, uint32_t number_of_blocks) {
  while (number_of_blocks--) {
    uint32_t arg = sdhc ? lba : lba * SD_BLOCK_SIZE;

    /* Read Single Block */
    if (prv_send_cmd(p, CMD17_READ_SINGLE, arg, 0xFF) != 0) {
      prv_cs_high(p);
      LOG_DEBUG("sd_card: Read Single Block (CMD17) failed\n");
      return STATUS_CODE_INTERNAL_ERROR;
    }

    /* wait start token */
    uint32_t start = HAL_GetTick();
    /* Keeps checking if data is available to read */
    while (prv_transfer_byte(p, 0xFF) == 0xFF) {
      if (HAL_GetTick() - start > T_RW_MS) {
        prv_cs_high(p);
        LOG_DEBUG("sd_card: Read Single Block (CMD17) timed out\n");
        return STATUS_CODE_TIMEOUT;
      }
    }

    /* Read block!! */
    prv_txrx(p, dst, dst, SD_BLOCK_SIZE);
    uint8_t junk[2];
    prv_txrx(p, junk, junk, 2);

    /* Update Buffer and block pointer */
    dst += SD_BLOCK_SIZE;
    lba++;
    prv_cs_high(p);
  }
  return STATUS_CODE_OK;
}

StatusCode sd_write_blocks(SpiPort p, const uint8_t *src, uint32_t lba, uint32_t number_of_blocks) {
 uint32_t arg = sdhc ? lba : lba * SD_BLOCK_SIZE;

  if (number_of_blocks == 1) {
    if (prv_send_cmd(p, CMD24_WRITE_SINGLE, arg, 0xFF) != 0) {
      prv_cs_high(p);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    prv_transfer_byte(p, SD_TOKEN_START_BLOCK);
    prv_txrx(p, (uint8_t *)src, (uint8_t *)src, SD_BLOCK_SIZE);
    uint8_t crc[2] = { 0xFF, 0xFF };
    prv_txrx(p, crc, crc, 2);

    if ((prv_transfer_byte(p, 0xFF) & 0x1F) != DATA_ACCEPTED) {
      prv_cs_high(p);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    if (prv_wait_ready(p, T_RW_MS)) {
      prv_cs_high(p);
      return STATUS_CODE_TIMEOUT;
    }
    prv_cs_high(p);
    return STATUS_CODE_OK;
  }

  /* multi-block */
  prv_send_cmd(p, CMD55_APP_CMD, 0, 0xFF);
  prv_send_cmd(p, ACMD23_PRE_ERASE, number_of_blocks, 0xFF);

  if (prv_send_cmd(p, CMD25_WRITE_MULTI, arg, 0xFF) != 0) {
    prv_cs_high(p);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  while (number_of_blocks--) {
    prv_transfer_byte(p, SD_TOKEN_MULTI_WRITE);
    prv_txrx(p, (uint8_t *)src, (uint8_t *)src, SD_BLOCK_SIZE);
    uint8_t crc[2] = { 0xFF, 0xFF };
    prv_txrx(p, crc, crc, 2);

    if ((prv_transfer_byte(p, 0xFF) & 0x1F) != DATA_ACCEPTED) {
      prv_cs_high(p);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    if (prv_wait_ready(p, T_RW_MS)) {
      prv_cs_high(p);
      return STATUS_CODE_TIMEOUT;
    }

    src += SD_BLOCK_SIZE;
    lba++;
  }

  prv_transfer_byte(p, SD_TOKEN_STOP_TRAN);
  if (prv_wait_ready(p, T_RW_MS)) {
    prv_cs_high(p);
    return STATUS_CODE_TIMEOUT;
  }
  prv_cs_high(p);
  return STATUS_CODE_OK;
}

StatusCode sd_is_initialized(SpiPort p)
{
    /* is the card still busy with a write? */
    if (prv_wait_ready(p, 10) != 0) {          
        return STATUS_CODE_RESOURCE_EXHAUSTED; /* let FatFs retry later */
    }

    /* Ask the card for its status (CMD13 → R2) */
    uint8_t r1 = prv_send_cmd(p, CMD13_SEND_STATUS, 0, 0xFF); 
    uint8_t r2 = prv_xfer_byte(p, 0xFF);                     

    prv_cs_high(p);                                          
    prv_xfer_byte(p, 0xFF);                                 

    return (r1 == 0 && r2 == 0) ? STATUS_CODE_OK : STATUS_CODE_INTERNAL_ERROR;
}
