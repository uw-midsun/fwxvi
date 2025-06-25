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

/* Macros and Defines*/
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

#define SD_NUM_RETRIES 8

#define ACMD41_ARG_V2 0x40000000U
#define ACMD41_ARG_V1 0U

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

/* Declarations */
static inline void prv_sd_cs_low(SdSpiPort p);
static inline void prv_sd_cs_high(SdSpiPort p); 
void sd_spi_full_duplex_exchange(SdSpiPort spi, uint8_t *tx_buf, uint16_t tx_len, uint8_t *rx_buf, uint16_t rx_len);
static void prv_sd_txrx(SdSpiPort spi, uint8_t *tx, uint8_t *rx, uint16_t len);
static inline uint8_t prv_sd_transfer_byte(SdSpiPort p, uint8_t b); 
static uint32_t prv_sd_wait_ready(SdSpiPort p, uint32_t timeout_ms); 
static uint8_t prv_send_cmd(SdSpiPort p, uint8_t cmd, uint32_t arg, uint8_t crc);
static StatusCode prv_sd_spi_freq(SdSpiPort p, uint32_t baudrate_hz); 
static StatusCode prv_sd_spi_pins_config(SdSpiPort p, SdSpiSettings *settings);

/* Handles for each SPI port */
static SPI_HandleTypeDef hspi1;

/* Cache the user settings so cs pin etc. can be driven later */
static SdSpiSettings *g_settings[NUM_SPI_PORTS] = { 0 };

/* CS driven by settings->cs */
static inline void prv_sd_cs_low(SdSpiPort p) {
  gpio_set_state(&g_settings[p]->cs, GPIO_STATE_LOW);
}
static inline void prv_sd_cs_high(SdSpiPort p) {
  gpio_set_state(&g_settings[p]->cs, GPIO_STATE_HIGH);
}

void sd_spi_full_duplex_exchange(SdSpiPort spi, uint8_t *tx_buf, uint16_t tx_len, uint8_t *rx_buf, uint16_t rx_len) {
  uint16_t len = tx_len ? tx_len : rx_len;
  static const uint8_t ff = 0xFF;
  const uint8_t *tx = tx_buf ? tx_buf : &ff;
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tx, rx_buf, len, HAL_MAX_DELAY);
}

static void prv_sd_txrx(SdSpiPort spi, uint8_t *tx, uint8_t *rx, uint16_t len) {
  if (tx) {
    sd_spi_full_duplex_exchange(spi, tx, len, rx, len);
  } else {
    static uint8_t ff = 0xFF;
    for (uint16_t i = 0; i < len; i++) {
      sd_spi_full_duplex_exchange(spi, &ff, 1, &rx[i], 1);
    }
  }
}

static inline uint8_t prv_sd_transfer_byte(SdSpiPort p, uint8_t b) {
  uint8_t r;
  prv_sd_txrx(p, &b, &r, 1);
  return r;
}

static uint32_t prv_sd_wait_ready(SdSpiPort p, uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick();
  while (HAL_GetTick() - start < timeout_ms) {
    if (prv_sd_transfer_byte(p, 0xFF) == 0xFF) {
      return 0;
    }
  }
  LOG_DEBUG("sd_card: wait ready timed out\n");
  return 1;
}

static uint8_t prv_send_cmd(SdSpiPort p, uint8_t cmd, uint32_t arg, uint8_t crc) {
  uint8_t frame[6] = { CMD(cmd), 
                      (uint8_t)(arg >> 24), 
                      (uint8_t)(arg >> 16), 
                      (uint8_t)(arg >> 8), 
                      (uint8_t)(arg), 
                      crc };
  prv_sd_cs_low(p);
  prv_sd_wait_ready(p, T_RW_MS);
  sd_spi_full_duplex_exchange(p, frame, 6, frame, 6);
  for (uint8_t i = 0; i < 10; i++) {
    uint8_t r = prv_sd_transfer_byte(p, 0xFF);
    if (!(r & 0x80)) return r;
  }
  LOG_DEBUG("sd_card: no response to CMD%d\n", cmd);
  return 0xFF;
}

static bool sdhc = false;

static StatusCode prv_sd_spi_freq(SdSpiPort p, uint32_t baudrate_hz){
  SPI_HandleTypeDef *hspi = &hspi1; 
   __HAL_RCC_SPI1_CLK_ENABLE();
  uint32_t clk_freq = HAL_RCC_GetPCLK2Freq(); /* Only using SPI1, on APB2 Bus */

  /* SPI prescaler */
  size_t index = 32 - (size_t)__builtin_clz(clk_freq / baudrate_hz);
  uint16_t prescaler = (index - 1) << 3;
  if (!IS_SPI_BAUDRATE_PRESCALER(prescaler)){
    LOG_DEBUG("Invalid baud rate");
    return STATUS_CODE_INVALID_ARGS;
  }
  __HAL_SPI_DISABLE(hspi);                      
  MODIFY_REG(hspi->Instance->CR1, SPI_CR1_BR, prescaler); /* Note: if compiler error, try ignoring -Wsign-conversion errror (done in fwxv)*/
  __HAL_SPI_ENABLE(hspi);    
  return STATUS_CODE_OK;
}

static StatusCode prv_sd_spi_pins_config(SdSpiPort p, SdSpiSettings *settings){
  g_settings[p] = settings;

  /* Enabling SPI peripheral clock */
  __HAL_RCC_SPI1_CLK_ENABLE();;
  uint32_t clk_freq = HAL_RCC_GetPCLK2Freq(); /* Only using SPI1, on APB2 Bus */

  /* init GPIO pins from settings */
  gpio_init_pin_af(&settings->mosi, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin_af(&settings->miso, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin_af(&settings->sclk, GPIO_ALTFN_PUSH_PULL, GPIO_ALT5_SPI1);
  gpio_init_pin(&settings->cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  /* SPI prescaler */
  size_t index = 32 - (size_t)__builtin_clz(clk_freq / settings->baudrate);
  uint16_t prescaler = (index - 1) << 3;
  if (!IS_SPI_BAUDRATE_PRESCALER(prescaler)){
    LOG_DEBUG("Invalid baud rate");
    return STATUS_CODE_INVALID_ARGS;
  }

  /* SPI instance + params */
  SPI_HandleTypeDef *hspi = &hspi1;

  hspi->Init.Mode = SPI_MODE_MASTER;
  hspi->Init.Direction = SPI_DIRECTION_2LINES;
  hspi->Init.DataSize = SPI_DATASIZE_8BIT;
  hspi->Init.CLKPolarity = (settings->mode & 0x02) ? SPI_POLARITY_HIGH : SPI_POLARITY_LOW; 
  hspi->Init.CLKPhase = (settings->mode & 0x01) ? SPI_PHASE_2EDGE : SPI_PHASE_1EDGE; 
  hspi->Init.NSS = SPI_NSS_SOFT;
  hspi->Init.BaudRatePrescaler = (index - 2) << 3; 
  hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi->Init.TIMode = SPI_TIMODE_DISABLE; 
  hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi->Init.CRCPolynomial = 7;
  hspi->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  HAL_SPI_Init(hspi);

  return STATUS_CODE_OK;
}

/*  Public API 
------------------------------------------------------------------ */
StatusCode sd_card_spi_init(SdSpiPort p, SdSpiSettings *settings) {
  if (prv_sd_spi_pins_config(p, settings) != STATUS_CODE_OK){
    LOG_DEBUG("Error with pins config");
    return STATUS_CODE_INTERNAL_ERROR; 
  }

  /* 1. Set frequency to low */
  prv_sd_spi_freq(p, SD_SPI_INIT_LOW_FREQ_HZ);
  
  /* 2. 80 clocks with CS high */
  prv_sd_cs_high(p);
  for (uint8_t i = 0; i < 10; i++) {
    prv_sd_transfer_byte(p, 0xFF);
  }

  /* 3. Reset */
  uint8_t attempt;
  for(attempt = 0U; attempt < SD_NUM_RETRIES; ++attempt){
    /* Cleanup */
    prv_sd_cs_high(p);
    prv_sd_transfer_byte(p, 0xFF);

    /* If reset successful */
    if (prv_send_cmd(p, CMD0_GO_IDLE, 0, CRC_CMD0) == R1_IDLE) {
      LOG_DEBUG("SD card was successfully reset");
      break;
    }
    LOG_DEBUG("Failed to reset, retrying... attempt #%u", (attempt + 1)); 
    delay_ms(1U);
  }

  prv_sd_cs_high(p);
  prv_sd_transfer_byte(p, 0xFF);

  if (attempt >= SD_NUM_RETRIES){
    LOG_DEBUG("Retries didn't work, failed to reset");
    return STATUS_CODE_INTERNAL_ERROR; 
  }

  /* 4. Check SD card version */
  bool is_v2 = (prv_send_cmd(p, CMD8_SEND_IF_COND, 0x1AAU, CRC_CMD8) & R1_ILLEGAL) ? false : true;
  prv_sd_cs_high(p);
  prv_sd_transfer_byte(p, 0xFF);

  /* 5. Initiate init process w/ ACMD41 */
  uint32_t acmd41_arg = (is_v2) ? ACMD41_ARG_V2 : ACMD41_ARG_V1; 
  for(attempt = 0U; attempt < SD_NUM_RETRIES; ++attempt){
    /* Cleanup */
    prv_sd_cs_high(p);
    prv_sd_transfer_byte(p, 0xFF);

    /* About to send ACMD command */
    LOG_DEBUG("CMD55 returned %u", 
              prv_send_cmd(p, CMD55_APP_CMD, 0, 0xFFU)); 

    /* Send ACMD41 command */
    if (prv_send_cmd(p, ACMD41_SD_OP_COND, acmd41_arg, 0xFFU) == 0x00) {
      LOG_DEBUG("ACMD command was successfully sent");
      break;
    }
    LOG_DEBUG("Failed to send command, retrying... attempt #%u", (attempt + 1)); 
    delay_ms(1U);
  }

  prv_sd_cs_high(p); 
  prv_sd_transfer_byte(p, 0xFF); 

  if (attempt >= SD_NUM_RETRIES){
    LOG_DEBUG("Retries didn't work, failed to send ACMD command");
    return STATUS_CODE_INTERNAL_ERROR; 
  }

  /* 6. Read OCR */
  if(prv_send_cmd(p, CMD58_READ_OCR, 0U, 0xFFU) != 0x00) { 
    prv_sd_cs_high(p);
    LOG_DEBUG("CMD58 Read OCR failed..."); 
    return STATUS_CODE_INTERNAL_ERROR; 
  }
  uint8_t ocr[4];
  prv_sd_txrx(p, NULL, ocr, 4);  /* NULL tx -> send 0xFF dummy bytes, receive into ocr */
  bool is_sdhc = (ocr[0] & 0x40U) != 0; /*30th bit*/

  /* 7. No SDHC -> Block Length is 512 */
  if (!is_sdhc){
    if(prv_send_cmd(p, CMD16_SET_BLKLEN, 512U, 0xFFU) != 0x00){
      prv_sd_cs_high(p);
      LOG_DEBUG("CMD16 Block length set command failed"); 
      return STATUS_CODE_INTERNAL_ERROR;
    }
  }

  prv_sd_cs_high(p); 
  prv_sd_transfer_byte(p, 0xFF);
  
  /* 8. High Frequency is back */
  prv_sd_spi_freq(p, SD_SPI_INIT_HIGH_FREQ_HZ);
  
  return STATUS_CODE_OK;
}

StatusCode sd_read_blocks(SdSpiPort p, uint8_t *dst, uint32_t lba, uint32_t number_of_blocks) {
  while (number_of_blocks--) {
    uint32_t arg = sdhc ? lba : lba * SD_BLOCK_SIZE;

    /* Read Single Block */
    if (prv_send_cmd(p, CMD17_READ_SINGLE, arg, 0xFF) != 0) {
      prv_sd_cs_high(p);
      LOG_DEBUG("sd_card: Read Single Block (CMD17) failed\n");
      return STATUS_CODE_INTERNAL_ERROR;
    } 

    /* wait start token */
    uint32_t start = HAL_GetTick();
    /* Keeps checking if data is available to read */
    while (prv_sd_transfer_byte(p, 0xFF) == 0xFF) {
      if (HAL_GetTick() - start > T_RW_MS) {
        prv_sd_cs_high(p);
        LOG_DEBUG("sd_card: Read Single Block (CMD17) timed out\n");
        return STATUS_CODE_TIMEOUT;
      }
    }

    /* Read block!! */
    prv_sd_txrx(p, dst, dst, SD_BLOCK_SIZE);
    uint8_t junk[2];
    prv_sd_txrx(p, junk, junk, 2);

    /* Update Buffer and block pointer */
    dst += SD_BLOCK_SIZE;
    lba++;
    prv_sd_cs_high(p);
  }
  return STATUS_CODE_OK;
}

StatusCode sd_write_blocks(SdSpiPort p, uint8_t *src, uint32_t lba, uint32_t number_of_blocks) {
  uint32_t arg = sdhc ? lba : lba * SD_BLOCK_SIZE;

  if (number_of_blocks == 1) {
    if (prv_send_cmd(p, CMD24_WRITE_SINGLE, arg, 0xFF) != 0) {
      prv_sd_cs_high(p);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    prv_sd_transfer_byte(p, SD_TOKEN_START_BLOCK);
    prv_sd_txrx(p, (uint8_t *)src, (uint8_t *)src, SD_BLOCK_SIZE);
    uint8_t crc[2] = { 0xFF, 0xFF };
    prv_sd_txrx(p, crc, crc, 2);

    if ((prv_sd_transfer_byte(p, 0xFF) & 0x1F) != DATA_ACCEPTED) {
      prv_sd_cs_high(p);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    if (prv_sd_wait_ready(p, T_RW_MS)) {
      prv_sd_cs_high(p);
      return STATUS_CODE_TIMEOUT;
    }
    prv_sd_cs_high(p);
    return STATUS_CODE_OK;
  }

  /* multi-block */
  prv_send_cmd(p, CMD55_APP_CMD, 0, 0xFF);
  prv_send_cmd(p, ACMD23_PRE_ERASE, number_of_blocks, 0xFF);

  if (prv_send_cmd(p, CMD25_WRITE_MULTI, arg, 0xFF) != 0) {
    prv_sd_cs_high(p);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  while (number_of_blocks--) {
    prv_sd_transfer_byte(p, SD_TOKEN_MULTI_WRITE);
    prv_sd_txrx(p, (uint8_t *)src, (uint8_t *)src, SD_BLOCK_SIZE);
    uint8_t crc[2] = { 0xFF, 0xFF };
    prv_sd_txrx(p, crc, crc, 2);

    if ((prv_sd_transfer_byte(p, 0xFF) & 0x1F) != DATA_ACCEPTED) {
      prv_sd_cs_high(p);
      return STATUS_CODE_INTERNAL_ERROR;
    }
    if (prv_sd_wait_ready(p, T_RW_MS)) {
      prv_sd_cs_high(p);
      return STATUS_CODE_TIMEOUT;
    }

    src += SD_BLOCK_SIZE;
    lba++;
  }

  prv_sd_transfer_byte(p, SD_TOKEN_STOP_TRAN);
  if (prv_sd_wait_ready(p, T_RW_MS)) {
    prv_sd_cs_high(p);
    return STATUS_CODE_TIMEOUT;
  }
  prv_sd_cs_high(p);
  return STATUS_CODE_OK;
}

StatusCode sd_is_initialized(SdSpiPort p) {
  /* Is the card still busy with a write? */
  if (prv_sd_wait_ready(p, 10) != 0) {
    return STATUS_CODE_RESOURCE_EXHAUSTED; /* let FatFs retry later */
  }

  /* Ask the card for its status (CMD13 → R2) */
  uint8_t r1 = prv_send_cmd(p, CMD13_SEND_STATUS, 0, 0xFF);
  uint8_t r2 = prv_sd_transfer_byte(p, 0xFF);

  prv_sd_cs_high(p);
  prv_sd_transfer_byte(p, 0xFF);

  return (r1 == 0 && r2 == 0) ? STATUS_CODE_OK : STATUS_CODE_INTERNAL_ERROR;
}