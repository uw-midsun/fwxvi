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
#include "stm32l433xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_rcc.h"
#include "stm32l4xx_hal_spi.h"

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

/* Handles for each SPI port */
static SPI_HandleTypeDef hspi1;
static SPI_HandleTypeDef hspi2 __attribute__((alias("hspi1")));
static SPI_HandleTypeDef hspi3 __attribute__((alias("hspi1")));

/* Cache the user settings so cs pin etc. can be driven later */
static SdSpiSettings *g_settings[NUM_SPI_PORTS] = { 0 };

/* pull the right handle based on enum */
static SPI_HandleTypeDef *pick_hspi(SdSpiPort p) {
  return (p == SPI_PORT_1) ? &hspi1 : (p == SPI_PORT_2) ? &hspi2 : &hspi3;
}

/* CS driven by settings->cs */
static inline void prv_cs_low(SdSpiPort p) {
  gpio_set_state(&g_settings[p]->cs, GPIO_STATE_LOW);
}
static inline void prv_cs_high(SdSpiPort p) {
  gpio_set_state(&g_settings[p]->cs, GPIO_STATE_HIGH);
}

void spi_exchange(SdSpiPort spi, uint8_t *tx_buf, uint16_t tx_len, uint8_t *rx_buf, uint16_t rx_len) {
  uint16_t len = tx_len ? tx_len : rx_len;
  static const uint8_t ff = 0xFF;
  const uint8_t *tx = tx_buf ? tx_buf : &ff;
  HAL_SPI_TransmitReceive(pick_hspi(spi), (uint8_t *)tx, rx_buf, len, HAL_MAX_DELAY);
}

static void prv_txrx(SdSpiPort spi, uint8_t *tx, uint8_t *rx, uint16_t len) {
  if (tx) {
    spi_exchange(spi, tx, len, rx, len);
  } else {
    static uint8_t ff = 0xFF;
    for (uint16_t i = 0; i < len; i++) {
      spi_exchange(spi, &ff, 1, &rx[i], 1);
    }
  }
}

static inline uint8_t prv_transfer_byte(SdSpiPort p, uint8_t b) {
  uint8_t r;
  prv_txrx(p, &b, &r, 1);
  return r;
}

static uint32_t prv_wait_ready(SdSpiPort p, uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick();
  while (HAL_GetTick() - start < timeout_ms) {
    if (prv_transfer_byte(p, 0xFF) == 0xFF) {
      return 0;
    }
  }
  LOG_DEBUG("sd_card: wait ready timed out\n");
  return 1;
}

static uint8_t prv_send_cmd(SdSpiPort p, uint8_t cmd, uint32_t arg, uint8_t crc) {
  uint8_t frame[6] = { CMD(cmd), (uint8_t)(arg >> 24), (uint8_t)(arg >> 16), (uint8_t)(arg >> 8), (uint8_t)(arg), crc };
  prv_cs_low(p);
  prv_wait_ready(p, T_RW_MS);
  spi_exchange(p, frame, 6, frame, 6);
  for (uint8_t i = 0; i < 10; i++) {
    uint8_t r = prv_transfer_byte(p, 0xFF);
    if (!(r & 0x80)) return r;
  }
  LOG_DEBUG("sd_card: no response to CMD%d\n", cmd);
  return 0xFF;
}

static bool sdhc = false;

/*  Public API */

StatusCode sd_card_init(SdSpiPort p, SdSpiSettings *settings) {
  g_settings[p] = settings;

  /* init GPIO pins from settings */
  gpio_init_pin_af(&settings->mosi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin_af(&settings->miso, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  /* SPI instance + params */
  SPI_HandleTypeDef *h = pick_hspi(p);
  if (p == SPI_PORT_1)
    h->Instance = SPI1;
  else if (p == SPI_PORT_2)
    h->Instance = SPI2;
  else
    h->Instance = SPI3;

  h->Init.Mode = SPI_MODE_MASTER;
  h->Init.Direction = SPI_DIRECTION_2LINES;
  h->Init.DataSize = SPI_DATASIZE_8BIT;
  /* map CPOL/CPHA from settings->mode */
  switch (settings->mode) {
    case SD_SPI_MODE_0:
      h->Init.CLKPolarity = SPI_POLARITY_LOW;
      h->Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SD_SPI_MODE_1:
      h->Init.CLKPolarity = SPI_POLARITY_LOW;
      h->Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    case SD_SPI_MODE_2:
      h->Init.CLKPolarity = SPI_POLARITY_HIGH;
      h->Init.CLKPhase = SPI_PHASE_1EDGE;
      break;
    case SD_SPI_MODE_3:
      h->Init.CLKPolarity = SPI_POLARITY_HIGH;
      h->Init.CLKPhase = SPI_PHASE_2EDGE;
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }
  h->Init.NSS = SPI_NSS_SOFT;
  h->Init.BaudRatePrescaler = settings->baudrate;
  h->Init.FirstBit = SPI_FIRSTBIT_MSB;
  HAL_SPI_Init(h);

  /* 80 clocks with CS high */
  prv_cs_high(p);
  for (uint8_t i = 0; i < 10; i++) {
    prv_transfer_byte(p, 0xFF);
  }

  /* reset + CMD0… rest is unchanged */
  if (prv_send_cmd(p, CMD0_GO_IDLE, 0, CRC_CMD0) != R1_IDLE) {
    prv_cs_high(p);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  /* …voltage check, ACMD41 loop, CMD58→OCR, set sdhc… */

  return STATUS_CODE_OK;
}

StatusCode sd_read_blocks(SdSpiPort p, uint8_t *dst, uint32_t lba, uint32_t number_of_blocks) {
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

StatusCode sd_write_blocks(SdSpiPort p, uint8_t *src, uint32_t lba, uint32_t number_of_blocks) {
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

StatusCode sd_is_initialized(SdSpiPort p) {
  /* Is the card still busy with a write? */
  if (prv_wait_ready(p, 10) != 0) {
    return STATUS_CODE_RESOURCE_EXHAUSTED; /* let FatFs retry later */
  }

  /* Ask the card for its status (CMD13 → R2) */
  uint8_t r1 = prv_send_cmd(p, CMD13_SEND_STATUS, 0, 0xFF);
  uint8_t r2 = prv_transfer_byte(p, 0xFF);

  prv_cs_high(p);
  prv_transfer_byte(p, 0xFF);

  return (r1 == 0 && r2 == 0) ? STATUS_CODE_OK : STATUS_CODE_INTERNAL_ERROR;
}
