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
#include "FreeRTOS.h"
#include "delay.h"
#include "ff_gen_drv.h"
#include "log.h"
#include "task.h"

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

#define SD_SPI_INIT_LOW_FREQ SD_SPI_BAUDRATE_312_5KHZ
#define SD_SPI_HIGH_FREQ SD_SPI_BAUDRATE_2_5MHZ

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

// Keep FatFs path and filesystem state at file scope so they remain valid after mounting.
static char s_disk_path[4U] = { 0 };
static FATFS s_filesystem;
static bool s_driver_linked = false;

static SdSpiPort s_spi_port;
static SdSpiSettings *s_spi_settings;
static bool s_is_initialized = false;

/* SDHC/SDXC use block addressing; SDSC uses byte addressing */
static bool s_is_high_capacity = false;

/************************************************************************************************
 * Private helper functions
 ************************************************************************************************/

/* Time limits require task context. Confirm these margins on the fitted card. */
#define SD_READY_TIMEOUT_MS 1000U
#define SD_TOKEN_TIMEOUT_MS 500U
#define SD_INIT_TIMEOUT_MS 2000U
#define SD_RESPONSE_BYTES 8U

static bool s_transaction_active;

static StatusCode s_read_byte(uint8_t *out) {
  return sd_spi_rx(s_spi_port, out, 1U, SD_DUMMY_BYTE);
}

static StatusCode s_finish(StatusCode result) {
  if (s_transaction_active) {
    StatusCode end = sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
    s_transaction_active = false;
    if (result == STATUS_CODE_OK) result = end;
  }
  return result;
}

static StatusCode s_select(void) {
  StatusCode result = sd_spi_cs_set_state(s_spi_port, GPIO_STATE_LOW);
  if (result == STATUS_CODE_OK) s_transaction_active = true;
  return result;
}

static StatusCode s_wait_byte(uint8_t *out, bool ready, uint32_t timeout_ms) {
  TickType_t start = xTaskGetTickCount();
  TickType_t limit = pdMS_TO_TICKS(timeout_ms);
  if (!limit) limit = 1U;
  do {
    status_ok_or_return(s_read_byte(out));
    if (ready ? *out == 0xFFU : *out != 0xFFU) return STATUS_CODE_OK;
  } while ((TickType_t)(xTaskGetTickCount() - start) < limit);
  return STATUS_CODE_TIMEOUT;
}

static StatusCode s_wait_for_ready(void) {
  uint8_t byte;
  return s_wait_byte(&byte, true, SD_READY_TIMEOUT_MS);
}

/* A held transaction continues through data/CRC/programming. All other paths
 * finish here, preserving the first error even if release clocks also fail. */
static StatusCode s_send_sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc, SdResponseType expected, bool hold_cs, SdResponse *out) {
  *out = (SdResponse){ 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU };
  uint8_t frame[SD_SEND_SIZE] = { cmd | 0x40U, arg >> 24, arg >> 16, arg >> 8, arg, crc };
  StatusCode result = s_select();
  if (result != STATUS_CODE_OK) return result;
  result = s_wait_for_ready();
  if (result == STATUS_CODE_OK) result = sd_spi_tx(s_spi_port, frame, sizeof(frame));
  if (result != STATUS_CODE_OK) return s_finish(result);
  for (size_t i = 0; i < SD_RESPONSE_BYTES; ++i) {
    result = s_read_byte(&out->r1);
    if (result != STATUS_CODE_OK) return s_finish(result);
    if ((out->r1 & 0x80U) == 0U) break;
  }
  if (out->r1 & 0x80U) return s_finish(STATUS_CODE_TIMEOUT);
  if ((expected == SD_RESPONSE_R7 && out->r1 == SD_R1_IN_IDLE_STATE) || (expected == SD_RESPONSE_R3 && out->r1 == SD_R1_NO_ERROR)) {
    uint8_t bytes[4];
    result = sd_spi_rx(s_spi_port, bytes, sizeof(bytes), SD_DUMMY_BYTE);
    if (result == STATUS_CODE_OK) {
      out->r2 = bytes[0];
      out->r3 = bytes[1];
      out->r4 = bytes[2];
      out->r5 = bytes[3];
    }
  } else if (expected == SD_RESPONSE_R2) {
    result = s_read_byte(&out->r2);
  } else if (expected == SD_RESPONSE_R1B) {
    result = s_wait_for_ready();
  }
  if (result != STATUS_CODE_OK || !hold_cs) return s_finish(result);
  return result;
}

static uint32_t s_extension(const SdResponse *response) {
  return ((uint32_t)response->r2 << 24) | ((uint32_t)response->r3 << 16) | ((uint32_t)response->r4 << 8) | response->r5;
}

static DSTATUS sd_card_init(BYTE pdrv) {
  s_is_initialized = false;
  s_is_high_capacity = false;
  const char *stage = "transport";
  StatusCode result = sd_spi_init(s_spi_port, s_spi_settings);
  if (result != STATUS_CODE_OK) goto fail;
  result = sd_spi_set_frequency(s_spi_port, SD_SPI_INIT_LOW_FREQ);
  if (result != STATUS_CODE_OK) goto fail;
  stage = "power/clocks";
  delay_ms(10U);
  uint8_t clocks[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  result = sd_spi_tx(s_spi_port, clocks, sizeof(clocks));
  if (result != STATUS_CODE_OK) goto fail;
  stage = "CMD0";
  SdResponse response;
  TickType_t start = xTaskGetTickCount();
  do {
    result = s_send_sd_cmd(SD_CMD_GO_IDLE_STATE, 0U, 0x95U, SD_RESPONSE_R1, false, &response);
    if (result != STATUS_CODE_OK) goto fail;
    if (response.r1 == SD_R1_IN_IDLE_STATE) break;
    delay_ms(1U);
  } while ((TickType_t)(xTaskGetTickCount() - start) < pdMS_TO_TICKS(SD_INIT_TIMEOUT_MS));
  if (response.r1 != SD_R1_IN_IDLE_STATE) {
    result = STATUS_CODE_TIMEOUT;
    goto fail;
  }
  stage = "CMD8";
  result = s_send_sd_cmd(SD_CMD_SEND_IF_COND, 0x1AAU, 0x87U, SD_RESPONSE_R7, false, &response);
  if (result != STATUS_CODE_OK) goto fail;
  bool v2 = response.r1 == SD_R1_IN_IDLE_STATE;
  if ((v2 && (s_extension(&response) & 0xFFFU) != 0x1AAU) || (!v2 && response.r1 != (SD_R1_IN_IDLE_STATE | SD_R1_ILLEGAL_COMMAND))) {
    result = STATUS_CODE_INTERNAL_ERROR;
    goto fail;
  }
  stage = "CMD55/ACMD41";
  start = xTaskGetTickCount();
  do {
    result = s_send_sd_cmd(SD_CMD_APP_CMD, 0U, 0xFFU, SD_RESPONSE_R1, false, &response);
    if (result != STATUS_CODE_OK) goto fail;
    if (response.r1 != SD_R1_IN_IDLE_STATE) {
      result = STATUS_CODE_INTERNAL_ERROR;
      goto fail;
    }
    result = s_send_sd_cmd(SD_CMD_SD_APP_OP_COND, v2 ? 0x40000000U : 0U, 0xFFU, SD_RESPONSE_R1, false, &response);
    if (result != STATUS_CODE_OK) goto fail;
    if (response.r1 == SD_R1_NO_ERROR) break;
    if (response.r1 != SD_R1_IN_IDLE_STATE) {
      result = STATUS_CODE_INTERNAL_ERROR;
      goto fail;
    }
    delay_ms(10U);
  } while ((TickType_t)(xTaskGetTickCount() - start) < pdMS_TO_TICKS(SD_INIT_TIMEOUT_MS));
  if (response.r1 != SD_R1_NO_ERROR) {
    result = STATUS_CODE_TIMEOUT;
    goto fail;
  }
  stage = "CMD58";
  result = s_send_sd_cmd(SD_CMD_READ_OCR, 0U, 0xFFU, SD_RESPONSE_R3, false, &response);
  if (result != STATUS_CODE_OK) goto fail;
  uint32_t ocr = s_extension(&response);
  /* OCR voltage windows 20/21 cover 3.2–3.4 V on this 3.3 V board. */
  if (response.r1 != SD_R1_NO_ERROR || !(ocr & 0x80000000U) || !(ocr & (3UL << 20))) {
    result = STATUS_CODE_INTERNAL_ERROR;
    goto fail;
  }
  s_is_high_capacity = v2 && (ocr & 0x40000000U);
  if (!s_is_high_capacity) {
    stage = "CMD16";
    result = s_send_sd_cmd(SD_CMD_SET_BLOCKLEN, 512U, 0xFFU, SD_RESPONSE_R1, false, &response);
    if (result != STATUS_CODE_OK) goto fail;
    if (response.r1 != SD_R1_NO_ERROR) {
      result = STATUS_CODE_INTERNAL_ERROR;
      goto fail;
    }
  }
  stage = "operating clock";
  result = sd_spi_set_frequency(s_spi_port, SD_SPI_HIGH_FREQ);
  /* Apply and check the new settings before declaring the disk ready. */
  if (result == STATUS_CODE_OK) result = s_select();
  result = s_finish(result);
  if (result != STATUS_CODE_OK) goto fail;
  s_is_initialized = true;
  return 0U;
fail:
  result = s_finish(result);
  s_is_high_capacity = false;
  LOG_DEBUG("SD initialization failed at %s: %u\n", stage, (unsigned)result);
  return STA_NOINIT;
}

static DSTATUS sd_card_status(BYTE pdrv) {
  return s_is_initialized ? 0U : STA_NOINIT;
}

static uint32_t s_sd_block_addr(LBA_t sector) {
  return s_is_high_capacity ? (uint32_t)sector : (uint32_t)(sector * 512U);
}

static StatusCode s_read_data(uint8_t *data, size_t length) {
  uint8_t token, crc[2];
  status_ok_or_return(s_wait_byte(&token, false, SD_TOKEN_TIMEOUT_MS));
  if (token != SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) return STATUS_CODE_INTERNAL_ERROR;
  status_ok_or_return(sd_spi_rx(s_spi_port, data, length, SD_DUMMY_BYTE));
  return sd_spi_rx(s_spi_port, crc, sizeof(crc), SD_DUMMY_BYTE);
}

static DRESULT sd_read_blocks(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  if (!s_is_initialized) return RES_NOTRDY;
  if (!buff || !count) return RES_PARERR;
  while (count--) {
    SdResponse response;
    StatusCode result = s_send_sd_cmd(SD_CMD_READ_SINGLE_BLOCK, s_sd_block_addr(sector), 0xFFU, SD_RESPONSE_R1, true, &response);
    if (result == STATUS_CODE_OK && response.r1 != SD_R1_NO_ERROR) result = STATUS_CODE_INTERNAL_ERROR;
    if (result == STATUS_CODE_OK) result = s_read_data(buff, 512U);
    result = s_finish(result);
    if (result != STATUS_CODE_OK) return RES_ERROR;
    buff += 512U;
    ++sector;
  }
  return RES_OK;
}

static DRESULT sd_write_blocks(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  if (!s_is_initialized) return RES_NOTRDY;
  if (!buff || !count) return RES_PARERR;
  while (count--) {
    SdResponse response;
    StatusCode result = s_send_sd_cmd(SD_CMD_WRITE_SINGLE_BLOCK, s_sd_block_addr(sector), 0xFFU, SD_RESPONSE_R1, true, &response);
    if (result == STATUS_CODE_OK && response.r1 != SD_R1_NO_ERROR) result = STATUS_CODE_INTERNAL_ERROR;
    uint8_t prefix[] = { 0xFFU, SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE }, crc[] = { 0xFFU, 0xFFU }, token;
    if (result == STATUS_CODE_OK) result = sd_spi_tx(s_spi_port, prefix, sizeof(prefix));
    if (result == STATUS_CODE_OK) result = sd_spi_tx(s_spi_port, (uint8_t *)buff, 512U);
    if (result == STATUS_CODE_OK) result = sd_spi_tx(s_spi_port, crc, sizeof(crc));
    if (result == STATUS_CODE_OK) result = s_wait_byte(&token, false, SD_TOKEN_TIMEOUT_MS);
    if (result == STATUS_CODE_OK && (token & 0x1FU) != SD_DATA_OK) result = STATUS_CODE_INTERNAL_ERROR;
    if (result == STATUS_CODE_OK) result = s_wait_for_ready();
    result = s_finish(result);
    if (result != STATUS_CODE_OK) return RES_ERROR;
    buff += 512U;
    ++sector;
  }
  return RES_OK;
}

static DRESULT sd_card_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  if (!s_is_initialized) return RES_NOTRDY;
  if (cmd != CTRL_SYNC && !buff) return RES_PARERR;
  switch (cmd) {
    case CTRL_SYNC: {
      StatusCode result = s_select();
      if (result == STATUS_CODE_OK) result = s_wait_for_ready();
      return s_finish(result) == STATUS_CODE_OK ? RES_OK : RES_ERROR;
    }
    case GET_SECTOR_SIZE:
      *(WORD *)buff = 512U;
      return RES_OK;
    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1U;
      return RES_OK;
    case GET_SECTOR_COUNT: {
      SdResponse response;
      uint8_t csd[16];
      StatusCode result = s_send_sd_cmd(SD_CMD_SEND_CSD, 0U, 0xFFU, SD_RESPONSE_R1, true, &response);
      if (result == STATUS_CODE_OK && response.r1 != SD_R1_NO_ERROR) result = STATUS_CODE_INTERNAL_ERROR;
      if (result == STATUS_CODE_OK) result = s_read_data(csd, sizeof(csd));
      if (s_finish(result) != STATUS_CODE_OK) return RES_ERROR;
      uint64_t sectors;
      if ((csd[0] & 0xC0U) == 0x40U) {
        uint32_t c_size = ((uint32_t)(csd[7] & 0x3FU) << 16) | ((uint32_t)csd[8] << 8) | csd[9];
        sectors = ((uint64_t)c_size + 1U) * 1024U;
      } else if ((csd[0] & 0xC0U) == 0U) {
        uint8_t read_bl_len = csd[5] & 0x0FU;
        uint16_t c_size = ((csd[6] & 3U) << 10) | (csd[7] << 2) | (csd[8] >> 6);
        uint8_t mult = ((csd[9] & 3U) << 1) | (csd[10] >> 7);
        sectors = (((uint64_t)c_size + 1U) << (mult + 2U + read_bl_len)) / 512U;
      } else
        return RES_ERROR;
      if (!sectors || sectors > UINT32_MAX) return RES_ERROR;
      *(LBA_t *)buff = (LBA_t)sectors;
      return RES_OK;
    }
    default:
      return RES_PARERR;
  }
}

/************************************************************************************************
 * Public
 ************************************************************************************************/

// Link the SD card driver to FatFs and store the active SPI configuration.
StatusCode sd_card_link_driver(SdSpiPort spi, SdSpiSettings *settings) {
  if (settings == NULL || (unsigned)spi >= NUM_SD_SPI_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (s_driver_linked) {
    return STATUS_CODE_ALREADY_INITIALIZED;
  }

  if (FATFS_LinkDriver(&s_disk_driver, s_disk_path) != 0U) {
    LOG_DEBUG("Error linking SD card\n");
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_spi_settings = settings;
  s_spi_port = spi;
  s_driver_linked = true;

  LOG_DEBUG("SD card linked at: %s\n", s_disk_path);
  return STATUS_CODE_OK;
}

// Mount the SD card filesystem immediately using the registered driver.
FRESULT sd_card_mount(void) {
  if (!s_driver_linked) {
    return FR_INVALID_DRIVE;
  }

  FRESULT result = f_mount(&s_filesystem, s_disk_path, 1U);
  if (result != FR_OK) {
    LOG_DEBUG("SD card mount failed: %u\n", (unsigned)result);
    /* Unregister the filesystem object after a failed mount. */
    (void)f_mount(NULL, s_disk_path, 0U);
    return result;
  }

  LOG_DEBUG("SD card mounted at: %s\n", s_disk_path);
  return FR_OK;
}

const char *sd_card_drive_path(void) {
  return s_driver_linked ? s_disk_path : NULL;
}
