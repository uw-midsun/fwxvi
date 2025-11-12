/************************************************************************************************
 * @file    sd_card_interface.c
 *
 * @brief   Sd Card Interface
 *
 * @date    2025-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "delay.h"
#include "ff_gen_drv.h"
#include "log.h"

/* Intra-component Headers */
#include "sd_card_interface.h"

/**
 * @brief   SD Card command transmit size
 * @details 1 byte CMD index
 *          4 bytes CMD argument
 *          1 byte CRC7
 */
#define SD_SEND_SIZE 6U

/**
 * @brief   Number of retries during SD Card initialization
 */
#define SD_NUM_RETRIES 100U

/**
 * @brief   SD Card default dummy byte
 */
#define SD_DUMMY_BYTE 0xFFU

#define SD_SPI_INIT_LOW_FREQ_HZ SD_SPI_BAUDRATE_312_5KHZ
#define SD_SPI_HIGH_FREQ_HZ SD_SPI_BAUDRATE_2_5MHZ

#define SD_R1_NO_ERROR (0x00)
#define SD_R1_IN_IDLE_STATE (0x01)
#define SD_R1_ILLEGAL_COMMAND (0x04)

#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ 0xFEU
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE 0xFEU
#define SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE 0xFCU
#define SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE 0xFDU

#define SD_CMD_GO_IDLE_STATE 0U
#define SD_CMD_SEND_OP_COND 1U
#define SD_CMD_SEND_IF_COND 8U
#define SD_CMD_SEND_CSD 9U
#define SD_CMD_SEND_CID 10U
#define SD_CMD_STATUS 13U
#define SD_CMD_SET_BLOCKLEN 16U
#define SD_CMD_READ_SINGLE_BLOCK 17U
#define SD_CMD_READ_MULTIPLE_BLOCK 18U
#define SD_CMD_WRITE_SINGLE_BLOCK 24U
#define SD_CMD_WRITE_MULTI_BLOCK 25U
#define SD_CMD_SD_APP_OP_COND 41U
#define SD_CMD_APP_CMD 55U
#define SD_CMD_READ_OCR 58U

#define SD_DATA_OK 0x05U
#define SD_DATA_CRC_ERROR 0x0BU
#define SD_DATA_WRITE_ERROR 0x0DU
#define SD_DATA_OTHER_ERROR 0xFFU

/************************************************************************************************
 * SD Card responses
 ************************************************************************************************/

typedef enum { SD_RESPONSE_R1 = 0, SD_RESPONSE_R1B, SD_RESPONSE_R2, SD_RESPONSE_R3, SD_RESPONSE_R4R5, SD_RESPONSE_R7, NUM_SD_RESPONSES } SdResponseType;

typedef struct {
  uint8_t r1;
  uint8_t r2;
  uint8_t r3;
  uint8_t r4;
  uint8_t r5;
} SdResponse;

/************************************************************************************************
 * Driver function declarations
 ************************************************************************************************/
static DSTATUS sd_card_init(BYTE pdrv);
static DSTATUS sd_card_status(BYTE pdrv);
static DRESULT sd_read_blocks(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count);
static DRESULT sd_write_blocks(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count);
static DRESULT sd_card_ioctl(BYTE pdrv, BYTE cmd, void *buff);

/************************************************************************************************
 * Private variables
 ************************************************************************************************/
static Diskio_drvTypeDef s_disk_driver = {
  .disk_initialize = sd_card_init,
  .disk_status = sd_card_status,
  .disk_write = sd_write_blocks,
  .disk_read = sd_read_blocks,
  .disk_ioctl = sd_card_ioctl,
};

static char s_disk_path[4U] = { "/sd" };

static SdSpiPort s_spi_port;
static SdSpiSettings *s_spi_settings;
static bool s_is_initialized = false;

/************************************************************************************************
 * Private helper functions
 ************************************************************************************************/

static uint8_t s_read_byte(void) {
  uint8_t result = 0x00U;
  sd_spi_rx(s_spi_port, &result, 1U, 0xFFU);
  return result;
}

static void s_clock_dummy_bytes(uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    s_read_byte();
  }
}

static uint8_t s_full_duplex_transfer(uint8_t byte) {
  uint8_t result = 0x00;
  sd_spi_rx(s_spi_port, &result, 1U, byte);
  return result;
}

static bool s_wait_for_ready(void) {
  uint16_t timeout = 0xFFFFU;
  uint8_t readvalue;

  do {
    readvalue = s_full_duplex_transfer(SD_DUMMY_BYTE);
    if (--timeout == 0U) {
      return false;
    }
  } while (readvalue != 0xFFU);

  return true;
}

static uint8_t s_wait_for_response(void) {
  uint16_t timeout = 0xFFFFU;
  uint8_t readvalue;

  do {
    readvalue = s_full_duplex_transfer(SD_DUMMY_BYTE);
    if (--timeout == 0U) {
      return 0xFFU;
    }
  } while (readvalue == SD_DUMMY_BYTE);

  return readvalue;
}

static SdResponse s_send_sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc, SdResponseType expected) {
  uint8_t frame[SD_SEND_SIZE];

  frame[0] = (cmd | 0x40);
  frame[1] = (uint8_t)(arg >> 24);
  frame[2] = (uint8_t)(arg >> 16);
  frame[3] = (uint8_t)(arg >> 8);
  frame[4] = (uint8_t)(arg);
  frame[5] = (uint8_t)(crc);

  sd_spi_cs_set_state(s_spi_port, GPIO_STATE_LOW);

  if (!s_wait_for_ready()) {
    sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
    s_read_byte();
    return ((SdResponse){ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF });
  }

  sd_spi_tx(s_spi_port, frame, SD_SEND_SIZE);

  SdResponse res = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  switch (expected) {
    case SD_RESPONSE_R1:
      res.r1 = s_wait_for_response();
      break;
    case SD_RESPONSE_R1B:
      res.r1 = s_wait_for_response();
      res.r2 = s_full_duplex_transfer(SD_DUMMY_BYTE);
      sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
      delay_ms(1);
      sd_spi_cs_set_state(s_spi_port, GPIO_STATE_LOW);
      while (s_full_duplex_transfer(SD_DUMMY_BYTE) != 0xFFU) {
      }
      break;
    case SD_RESPONSE_R2:
      res.r1 = s_wait_for_response();
      res.r2 = s_full_duplex_transfer(SD_DUMMY_BYTE);
      break;
    case SD_RESPONSE_R3:
    case SD_RESPONSE_R7:
      res.r1 = s_wait_for_response();
      res.r2 = s_full_duplex_transfer(SD_DUMMY_BYTE);
      res.r3 = s_full_duplex_transfer(SD_DUMMY_BYTE);
      res.r4 = s_full_duplex_transfer(SD_DUMMY_BYTE);
      res.r5 = s_full_duplex_transfer(SD_DUMMY_BYTE);
      break;
    default:
      break;
  }

  if (expected != SD_RESPONSE_R1B) {
    sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
    s_read_byte();
  }

  return res;
}

static StatusCode s_sd_get_data_response(void) {
  uint8_t dataresponse;
  uint16_t timeout = 0xFFFFU;

  do {
    dataresponse = s_read_byte();
    if (--timeout == 0U) {
      sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
      return STATUS_CODE_TIMEOUT;
    }
  } while (dataresponse == 0xFFu);

  s_read_byte();

  if ((dataresponse & 0x1F) == SD_DATA_OK) {
    sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
    sd_spi_cs_set_state(s_spi_port, GPIO_STATE_LOW);

    while (s_read_byte() != 0xFFU) {
    }
    return STATUS_CODE_OK;
  }

  sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
  return STATUS_CODE_INTERNAL_ERROR;
}

/************************************************************************************************
 * SD Card driver functions
 ************************************************************************************************/

static DSTATUS sd_card_init(BYTE pdrv) {
  if (sd_spi_init(s_spi_port, s_spi_settings) != STATUS_CODE_OK) {
    return RES_ERROR;
  }

  /* Step 0: Initialize at a low frequency */
  sd_spi_set_frequency(s_spi_port, SD_SPI_INIT_LOW_FREQ_HZ);

  sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);

  /* Step 1: Send 80 clock cycles with CS high to wake up card */
  /* 10 * 8 bits = 80 clocked bits */
  s_clock_dummy_bytes(10U);

  delay_ms(10);

  /* Step 2: reset */
  SdResponse r1;

  for (uint8_t i = 0U; i < SD_NUM_RETRIES; i++) {
    r1 = s_send_sd_cmd(SD_CMD_GO_IDLE_STATE, 0U, 0x95U, SD_RESPONSE_R1);

    if (r1.r1 == SD_R1_IN_IDLE_STATE) {
      break;
    }

    delay_ms(1U);
  }

  if (r1.r1 != SD_R1_IN_IDLE_STATE) {
    LOG_DEBUG("Failed to reset SD card\n");
    sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
    s_is_initialized = false;
    return RES_ERROR;
  }

  LOG_DEBUG("Resetted SD card\n");

  /* Step 3: Check SD version with CMD8 */
  r1 = s_send_sd_cmd(SD_CMD_SEND_IF_COND, 0x1AAU, 0x87U, SD_RESPONSE_R7);
  bool is_v2 = (r1.r1 & SD_R1_ILLEGAL_COMMAND) == 0U ? true : false;

  /* Step 4: Initiate init process with ACMD41 */
  uint32_t acmd41_arg = is_v2 ? 0x40000000U : 0U;
  for (uint16_t i = 0U; i < SD_NUM_RETRIES; i++) {
    s_send_sd_cmd(SD_CMD_APP_CMD, 0U, 0xFFU, SD_RESPONSE_R1);
    r1 = s_send_sd_cmd(SD_CMD_SD_APP_OP_COND, acmd41_arg, 0xFFU, SD_RESPONSE_R1);

    if (r1.r1 == SD_R1_NO_ERROR) {
      break;
    }

    delay_ms(1U);
  }

  if (r1.r1 != SD_R1_NO_ERROR) {
    LOG_DEBUG("Failed to initiate init process with ACMD41 cmd\n");
    sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
    s_is_initialized = false;
    return RES_ERROR;
  }

  /* Step 5: Read OCR */
  r1 = s_send_sd_cmd(SD_CMD_READ_OCR, 0U, 0xFFU, SD_RESPONSE_R3);
  bool is_sdhc = (r1.r2 & 0x40U);

  /* Step 6: If not SDHC set block length to 512 */
  if (!is_sdhc) {
    r1 = s_send_sd_cmd(SD_CMD_SET_BLOCKLEN, 512U, 0xFFU, SD_RESPONSE_R1);

    if (r1.r1 != SD_R1_NO_ERROR) {
      sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
      s_is_initialized = false;
      return RES_ERROR;
    }
  }

  sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
  s_read_byte();

  /* Step 7: Reinitialize at a high frequency */
  sd_spi_set_frequency(s_spi_port, SD_SPI_HIGH_FREQ_HZ);

  s_is_initialized = true;
  return RES_OK;
}

static DSTATUS sd_card_status(BYTE pdrv) {
  return s_is_initialized ? RES_OK : RES_NOTRDY;
}

static DRESULT sd_read_blocks(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  DRESULT result = RES_OK;

  while (count--) {
    SdResponse r1 = s_send_sd_cmd(SD_CMD_READ_SINGLE_BLOCK, sector * 512U, 0xFFU, SD_RESPONSE_R1);

    if (r1.r1 != SD_R1_NO_ERROR) {
      result = RES_ERROR;
      break;
    }

    uint8_t token;
    uint16_t timeout = 0xFFFFU;

    do {
      token = s_read_byte();
      if (--timeout == 0U) {
        LOG_DEBUG("Timeout waiting for data token\n");
        result = RES_ERROR;
        break;
      }
    } while (token == 0xFFU);

    if (result != RES_OK || token != SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) {
      sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
      s_read_byte();
      return RES_ERROR;
    }

    for (uint16_t i = 0U; i < 512U; i++) {
      *buff++ = s_read_byte();
    }

    /* Discard CRC */
    s_read_byte();
    s_read_byte();

    sector++;
  }

  sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
  s_read_byte();

  return result;
}

static DRESULT sd_write_blocks(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  while (count--) {
    SdResponse r1 = s_send_sd_cmd(SD_CMD_WRITE_SINGLE_BLOCK, sector * 512U, 0xFFU, SD_RESPONSE_R1);

    if (r1.r1 != SD_R1_NO_ERROR) {
      return RES_ERROR;
    }

    s_read_byte();

    sd_spi_tx(s_spi_port, (uint8_t[]){ SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE }, 1);
    sd_spi_tx(s_spi_port, (uint8_t *)buff, 512);
    sd_spi_tx(s_spi_port, (uint8_t[]){ 0xFF, 0xFF }, 2);

    if (s_sd_get_data_response() != STATUS_CODE_OK) return RES_ERROR;

    buff += 512U;
    sector++;
  }

  sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
  s_read_byte();
  return RES_OK;
}

static DRESULT sd_card_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  switch (cmd) {
    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_SIZE:
      *(WORD *)buff = 512U;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1U;
      return RES_OK;

    case GET_SECTOR_COUNT: {
      uint8_t csd[16];
      uint32_t sector_count = 0;

      SdResponse r1 = s_send_sd_cmd(SD_CMD_SEND_CSD, 0U, 0xFFU, SD_RESPONSE_R1);
      if (r1.r1 != SD_R1_NO_ERROR) {
        *(DWORD *)buff = 32768U;
        return RES_OK;
      }

      uint16_t timeout = 0xFFFF;
      uint8_t token;
      do {
        token = s_read_byte();
        if (--timeout == 0) {
          sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
          s_read_byte();
          *(DWORD *)buff = 32768;
          return RES_OK;
        }
      } while (token == 0xFF);

      if (token != SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) {
        sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
        s_read_byte();
        *(DWORD *)buff = 32768;
        return RES_OK;
      }

      for (int i = 0; i < 16; i++) {
        csd[i] = s_read_byte();
      }

      s_read_byte();
      s_read_byte();

      sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
      s_read_byte();

      if ((csd[0] & 0xC0) == 0x40) {
        uint32_t c_size = ((uint32_t)csd[7] & 0x3F) << 16;
        c_size |= (uint32_t)csd[8] << 8;
        c_size |= (uint32_t)csd[9];
        sector_count = (c_size + 1) * 1024;
      } else {
        uint8_t read_bl_len = csd[5] & 0x0F;
        uint16_t c_size = ((csd[6] & 0x03) << 10) | (csd[7] << 2) | ((csd[8] & 0xC0) >> 6);
        uint8_t c_size_mult = ((csd[9] & 0x03) << 1) | ((csd[10] & 0x80) >> 7);
        sector_count = (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
      }

      *(DWORD *)buff = sector_count;
      return RES_OK;
    }

    default:
      return RES_PARERR;
  }
}

/************************************************************************************************
 * Public
 ************************************************************************************************/

StatusCode sd_card_link_driver(SdSpiPort spi, SdSpiSettings *settings) {
  s_spi_settings = settings;
  s_spi_port = spi;

  if (FATFS_LinkDriver(&s_disk_driver, s_disk_path) == 0) {
    LOG_DEBUG("SD card linked at: %s\n", s_disk_path);
  } else {
    LOG_DEBUG("Error linking SD card\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}
