/************************************************************************************************
 * @file    sd_card_interface.c
 *
 * @brief   Sd Card Interface
 *          Good resource to check out (go to section 7) https://academy.cba.mit.edu/classes/networking_communications/SD/SD.pdf
 *
 * @date    2025-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

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
 * @brief   SD Card default dummy byte
 */
#define SD_DUMMY_BYTE 0xFFU

#define SD_SPI_INIT_LOW_FREQ SD_SPI_BAUDRATE_312_5KHZ
#define SD_SPI_HIGH_FREQ SD_SPI_BAUDRATE_2_5MHZ

#define SD_R1_NO_ERROR 0x00U
#define SD_R1_IN_IDLE_STATE 0x01U
#define SD_R1_ILLEGAL_COMMAND 0x04U

#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ 0xFEU
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE 0xFEU

/**
 * @brief   SD memory-card commands supported in SPI mode.
 * @details SD Physical Layer Specification v3.01, Tables 7-3 and 7-4. Reserved,
 *          SD-only, and SDIO commands are omitted. Optional commands still
 *          depend on card support; entries do not imply driver implementation.
 */
typedef enum {
  SD_CMD0_GO_IDLE_STATE = 0U,
  SD_CMD1_SEND_OP_COND = 1U,
  SD_CMD6_SWITCH_FUNC = 6U,
  SD_CMD8_SEND_IF_COND = 8U,
  SD_CMD9_SEND_CSD = 9U,
  SD_CMD10_SEND_CID = 10U,
  SD_CMD12_STOP_TRANSMISSION = 12U,
  SD_CMD13_SEND_STATUS = 13U,
  SD_CMD16_SET_BLOCKLEN = 16U,
  SD_CMD17_READ_SINGLE_BLOCK = 17U,
  SD_CMD18_READ_MULTIPLE_BLOCK = 18U,
  SD_CMD24_WRITE_SINGLE_BLOCK = 24U,
  SD_CMD25_WRITE_MULTIPLE_BLOCK = 25U,
  SD_CMD27_PROGRAM_CSD = 27U,
  SD_CMD28_SET_WRITE_PROT = 28U,  /* SDSC only */
  SD_CMD29_CLR_WRITE_PROT = 29U,  /* SDSC only */
  SD_CMD30_SEND_WRITE_PROT = 30U, /* SDSC only */
  SD_CMD32_ERASE_WR_BLK_START_ADDR = 32U,
  SD_CMD33_ERASE_WR_BLK_END_ADDR = 33U,
  SD_CMD38_ERASE = 38U,
  SD_CMD42_LOCK_UNLOCK = 42U,
  SD_CMD55_APP_CMD = 55U,
  SD_CMD56_GEN_CMD = 56U,
  SD_CMD58_READ_OCR = 58U,
  SD_CMD59_CRC_ON_OFF = 59U,

  /* Application commands require CMD55 first */
  SD_ACMD13_SD_STATUS = 13U,
  SD_ACMD22_SEND_NUM_WR_BLOCKS = 22U,
  SD_ACMD23_SET_WR_BLK_ERASE_COUNT = 23U,
  SD_ACMD41_SD_SEND_OP_COND = 41U,
  SD_ACMD42_SET_CLR_CARD_DETECT = 42U,
  SD_ACMD51_SEND_SCR = 51U,
} SdCommand;

#define SD_DATA_OK 0x05U
#define SD_BLOCK_SIZE 512U

#define SD_READY_TIMEOUT_MS 1000U
#define SD_TOKEN_TIMEOUT_MS 500U
#define SD_INIT_TIMEOUT_MS 2000U
#define SD_RESPONSE_BYTES 8U

/************************************************************************************************
 * SD Card responses
 ************************************************************************************************/

typedef enum { SD_RESPONSE_R1 = 0, SD_RESPONSE_R3, SD_RESPONSE_R7 } SdResponseType;

typedef struct {
  uint8_t r1;
  uint8_t extension[4];
} SdResponse;

/************************************************************************************************
 * Driver function declarations
 ************************************************************************************************/
static DSTATUS sd_card_init(BYTE pdrv);
static DSTATUS sd_card_status(BYTE pdrv);
static DRESULT sd_read_blocks(BYTE pdrv, BYTE *buffer, LBA_t sector, UINT count);
static DRESULT sd_write_blocks(BYTE pdrv, const BYTE *buffer, LBA_t sector, UINT count);
static DRESULT sd_card_ioctl(BYTE pdrv, BYTE command, void *buffer);

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
static bool s_transaction_active;

/************************************************************************************************
 * SPI transaction helpers
 ************************************************************************************************/

static StatusCode s_read_byte(uint8_t *data) {
  return sd_spi_rx(s_spi_port, data, 1U, SD_DUMMY_BYTE);
}

/** @brief Pull CS low and set flag  on success (flag needed by s_finish_transaction) */
static inline StatusCode s_select_card(void) {
  StatusCode status = sd_spi_cs_set_state(s_spi_port, GPIO_STATE_LOW);
  if (status == STATUS_CODE_OK) {
    s_transaction_active = true;
  }
  return status;
}

/** @brief Release chip select if active transaction */
static StatusCode s_finish_transaction(StatusCode status) {
  if (!s_transaction_active) {
    return status;
  }

  StatusCode release_status = sd_spi_cs_set_state(s_spi_port, GPIO_STATE_HIGH);
  s_transaction_active = false;
  return status == STATUS_CODE_OK ? release_status : status;
}

/** @brief Wait for 0xFF (card not busy) */
static StatusCode s_wait_for_ready(void) {
  TickType_t start = xTaskGetTickCount();
  do {
    uint8_t data;
    status_ok_or_return(s_read_byte(&data));
    if (data == SD_DUMMY_BYTE) {
      return STATUS_CODE_OK;
    }
  } while ((TickType_t)(xTaskGetTickCount() - start) < pdMS_TO_TICKS(SD_READY_TIMEOUT_MS));

  return STATUS_CODE_TIMEOUT;
}

/** @brief Skip 0xFF bytes and wait until an actual byte shows up */
static StatusCode s_wait_for_token(uint8_t *token) {
  TickType_t start = xTaskGetTickCount();
  do {
    status_ok_or_return(s_read_byte(token));
    if (*token != SD_DUMMY_BYTE) {
      return STATUS_CODE_OK;
    }
  } while ((TickType_t)(xTaskGetTickCount() - start) < pdMS_TO_TICKS(SD_TOKEN_TIMEOUT_MS));

  return STATUS_CODE_TIMEOUT;
}

static StatusCode s_transfer_command(SdCommand command, uint32_t argument, uint8_t crc, SdResponseType response_type, SdResponse *response) {
  uint8_t frame[SD_SEND_SIZE] = { command | 0x40U, argument >> 24, argument >> 16, argument >> 8, argument, crc };
  memset(response, SD_DUMMY_BYTE, sizeof(*response));

  status_ok_or_return(s_wait_for_ready());
  status_ok_or_return(sd_spi_tx(s_spi_port, frame, sizeof(frame)));

  for (size_t i = 0U; i < SD_RESPONSE_BYTES; ++i) {
    status_ok_or_return(s_read_byte(&response->r1));
    if ((response->r1 & 0x80U) == 0U) {
      break;
    }
  }
  if ((response->r1 & 0x80U) != 0U) {
    return STATUS_CODE_TIMEOUT;
  }

  /* An unsupported CMD8 returns only R1, do not clock in an R7 extension */
  bool has_r7 = response_type == SD_RESPONSE_R7 && response->r1 == SD_R1_IN_IDLE_STATE;
  bool has_r3 = response_type == SD_RESPONSE_R3 && response->r1 == SD_R1_NO_ERROR;
  if (has_r7 || has_r3) {
    return sd_spi_rx(s_spi_port, response->extension, sizeof(response->extension), SD_DUMMY_BYTE);
  }

  return STATUS_CODE_OK;
}

/**
 * @param   keep_selected To keep CS low after a successful command when data transfer must follow within same transaction
 */
static StatusCode s_send_command(SdCommand command, uint32_t argument, uint8_t crc, SdResponseType response_type, bool keep_selected, SdResponse *response) {
  status_ok_or_return(s_select_card());
  StatusCode status = s_transfer_command(command, argument, crc, response_type, response);
  if (status != STATUS_CODE_OK || !keep_selected) {
    return s_finish_transaction(status);
  }
  return STATUS_CODE_OK;
}

static uint32_t s_response_value(const SdResponse *response) {
  return ((uint32_t)response->extension[0] << 24) | ((uint32_t)response->extension[1] << 16) | ((uint32_t)response->extension[2] << 8) | response->extension[3];
}

/* Card Initialization */

/**
 * @details https://nodeloop.org/guides/sd-card-spi-init-guide/
 *          tldr send 80 clocks via 10 dummy bytes
 */
static StatusCode s_start_clock(void) {
  delay_ms(10U); /* settle */
  uint8_t clocks[10];
  memset(clocks, SD_DUMMY_BYTE, sizeof(clocks));
  return sd_spi_tx(s_spi_port, clocks, sizeof(clocks));
}

static StatusCode s_reset_card(void) {
  TickType_t start = xTaskGetTickCount();
  do {
    SdResponse response;
    status_ok_or_return(s_send_command(SD_CMD0_GO_IDLE_STATE, 0U, 0x95U, SD_RESPONSE_R1, false, &response));
    if (response.r1 == SD_R1_IN_IDLE_STATE) {
      return STATUS_CODE_OK;
    }
    delay_ms(1U);
  } while ((TickType_t)(xTaskGetTickCount() - start) < pdMS_TO_TICKS(SD_INIT_TIMEOUT_MS));

  return STATUS_CODE_TIMEOUT;
}

static StatusCode s_check_card_version(bool *version_two) {
  SdResponse response;
  status_ok_or_return(s_send_command(SD_CMD8_SEND_IF_COND, 0x1AAU, 0x87U, SD_RESPONSE_R7, false, &response));

  *version_two = response.r1 == SD_R1_IN_IDLE_STATE;
  if (*version_two) {
    /* The card must echo the supplied voltage and check pattern. */
    if ((s_response_value(&response) & 0xFFFU) != 0x1AAU) {
      return STATUS_CODE_INTERNAL_ERROR;
    }
  } else if (response.r1 != (SD_R1_IN_IDLE_STATE | SD_R1_ILLEGAL_COMMAND)) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

static StatusCode s_wait_for_initialization(bool version_two) {
  TickType_t start = xTaskGetTickCount();
  do {
    SdResponse response;
    status_ok_or_return(s_send_command(SD_CMD55_APP_CMD, 0U, SD_DUMMY_BYTE, SD_RESPONSE_R1, false, &response));
    if (response.r1 != SD_R1_IN_IDLE_STATE) {
      return STATUS_CODE_INTERNAL_ERROR;
    }

    uint32_t argument = version_two ? 0x40000000U : 0U;
    status_ok_or_return(s_send_command(SD_ACMD41_SD_SEND_OP_COND, argument, SD_DUMMY_BYTE, SD_RESPONSE_R1, false, &response));
    if (response.r1 == SD_R1_NO_ERROR) {
      return STATUS_CODE_OK;
    }
    if (response.r1 != SD_R1_IN_IDLE_STATE) {
      return STATUS_CODE_INTERNAL_ERROR;
    }
    delay_ms(10U);
  } while ((TickType_t)(xTaskGetTickCount() - start) < pdMS_TO_TICKS(SD_INIT_TIMEOUT_MS));

  return STATUS_CODE_TIMEOUT;
}

static StatusCode s_check_operating_conditions(bool version_two) {
  SdResponse response;
  status_ok_or_return(s_send_command(SD_CMD58_READ_OCR, 0U, SD_DUMMY_BYTE, SD_RESPONSE_R3, false, &response));

  uint32_t ocr = s_response_value(&response);
  /* Require power-up complete and a voltage window covering this 3.3 V board. */
  bool powered_up = (ocr & 0x80000000U) != 0U;
  bool supports_voltage = (ocr & (3UL << 20)) != 0U;
  if (response.r1 != SD_R1_NO_ERROR || !powered_up || !supports_voltage) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  s_is_high_capacity = version_two && (ocr & 0x40000000U) != 0U;
  return STATUS_CODE_OK;
}

static StatusCode s_set_block_size(void) {
  if (s_is_high_capacity) {
    return STATUS_CODE_OK;
  }

  SdResponse response;
  status_ok_or_return(s_send_command(SD_CMD16_SET_BLOCKLEN, SD_BLOCK_SIZE, SD_DUMMY_BYTE, SD_RESPONSE_R1, false, &response));
  return response.r1 == SD_R1_NO_ERROR ? STATUS_CODE_OK : STATUS_CODE_INTERNAL_ERROR;
}

static StatusCode s_set_operating_clock(void) {
  status_ok_or_return(sd_spi_set_frequency(s_spi_port, SD_SPI_HIGH_FREQ));
  return s_finish_transaction(s_select_card());
}

/** @brief Leave the disk unready and report which initialization step failed */
static inline DSTATUS s_initialization_failed(const char *stage, StatusCode status) {
  status = s_finish_transaction(status);
  s_is_high_capacity = false;
  LOG_DEBUG("SD initialization failed at %s: %u\n", stage, (unsigned)status);
  return STA_NOINIT;
}

static DSTATUS sd_card_init(BYTE pdrv) {
  s_is_initialized = false;
  s_is_high_capacity = false;

  StatusCode status = sd_spi_init(s_spi_port, s_spi_settings);
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("transport", status);
  }
  status = sd_spi_set_frequency(s_spi_port, SD_SPI_INIT_LOW_FREQ);
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("transport", status);
  }
  status = s_start_clock();
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("power/clocks", status);
  }
  status = s_reset_card();
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("CMD0", status);
  }

  bool version_two;
  status = s_check_card_version(&version_two);
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("CMD8", status);
  }
  status = s_wait_for_initialization(version_two);
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("CMD55/ACMD41", status);
  }
  status = s_check_operating_conditions(version_two);
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("CMD58", status);
  }
  status = s_set_block_size();
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("CMD16", status);
  }
  status = s_set_operating_clock();
  if (status != STATUS_CODE_OK) {
    return s_initialization_failed("operating clock", status);
  }

  s_is_initialized = true;
  return 0U;
}

/************************************************************************************************
 * Block transfers
 ************************************************************************************************/

static uint32_t s_block_address(LBA_t sector) {
  return s_is_high_capacity ? (uint32_t)sector : (uint32_t)(sector * SD_BLOCK_SIZE);
}

/** @brief Receive a data block and consume its CRC while CS remains low. */
static StatusCode s_read_data(uint8_t *data, size_t length) {
  uint8_t token;
  status_ok_or_return(s_wait_for_token(&token));
  if (token != SD_TOKEN_START_DATA_SINGLE_BLOCK_READ) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  uint8_t crc[2];
  status_ok_or_return(sd_spi_rx(s_spi_port, data, length, SD_DUMMY_BYTE));
  return sd_spi_rx(s_spi_port, crc, sizeof(crc), SD_DUMMY_BYTE);
}

/** @brief Read command data, releasing CS on both success and failure. */
static StatusCode s_read_command_data(SdCommand command, uint32_t argument, uint8_t *data, size_t length) {
  SdResponse response;
  status_ok_or_return(s_send_command(command, argument, SD_DUMMY_BYTE, SD_RESPONSE_R1, true, &response));
  if (response.r1 != SD_R1_NO_ERROR) {
    return s_finish_transaction(STATUS_CODE_INTERNAL_ERROR);
  }
  return s_finish_transaction(s_read_data(data, length));
}

/** @brief Send one block and wait for the card to finish programming it. */
static StatusCode s_write_data(const uint8_t *data) {
  uint8_t prefix[] = { SD_DUMMY_BYTE, SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE };
  uint8_t crc[] = { SD_DUMMY_BYTE, SD_DUMMY_BYTE };
  status_ok_or_return(sd_spi_tx(s_spi_port, prefix, sizeof(prefix)));
  status_ok_or_return(sd_spi_tx(s_spi_port, (uint8_t *)data, SD_BLOCK_SIZE));
  status_ok_or_return(sd_spi_tx(s_spi_port, crc, sizeof(crc)));

  uint8_t token;
  status_ok_or_return(s_wait_for_token(&token));
  if ((token & 0x1FU) != SD_DATA_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return s_wait_for_ready();
}

static StatusCode s_write_block(uint32_t address, const uint8_t *data) {
  SdResponse response;
  status_ok_or_return(s_send_command(SD_CMD24_WRITE_SINGLE_BLOCK, address, SD_DUMMY_BYTE, SD_RESPONSE_R1, true, &response));
  if (response.r1 != SD_R1_NO_ERROR) {
    return s_finish_transaction(STATUS_CODE_INTERNAL_ERROR);
  }
  return s_finish_transaction(s_write_data(data));
}

static StatusCode s_read_sector_count(LBA_t *sector_count) {
  uint8_t csd[16];
  status_ok_or_return(s_read_command_data(SD_CMD9_SEND_CSD, 0U, csd, sizeof(csd)));

  uint64_t sectors;
  if ((csd[0] & 0xC0U) == 0x40U) {
    /* SDHC/SDXC: C_SIZE in groups of 1024 sectors */
    uint32_t card_size = ((uint32_t)(csd[7] & 0x3FU) << 16) | ((uint32_t)csd[8] << 8) | csd[9];

    sectors = ((uint64_t)card_size + 1U) * 1024U;
  } else if ((csd[0] & 0xC0U) == 0U) {
    /* Version 2 */
    uint8_t block_length = csd[5] & 0x0FU;
    uint16_t card_size = ((csd[6] & 3U) << 10) | (csd[7] << 2) | (csd[8] >> 6);
    uint8_t multiplier = ((csd[9] & 3U) << 1) | (csd[10] >> 7);

    sectors = (((uint64_t)card_size + 1U) << (multiplier + 2U + block_length)) / SD_BLOCK_SIZE;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  if (sectors == 0U || sectors > UINT32_MAX) {
    return STATUS_CODE_OUT_OF_RANGE;
  }
  *sector_count = (LBA_t)sectors;
  return STATUS_CODE_OK;
}

/************************************************************************************************
 * FatFs callbacks
 ************************************************************************************************/

static DSTATUS sd_card_status(BYTE pdrv) {
  return s_is_initialized ? 0U : STA_NOINIT;
}

static DRESULT sd_read_blocks(BYTE pdrv, BYTE *buffer, LBA_t sector, UINT count) {
  if (!s_is_initialized) {
    return RES_NOTRDY;
  }
  if (buffer == NULL || count == 0U) {
    return RES_PARERR;
  }

  for (UINT i = 0U; i < count; ++i) {
    if (s_read_command_data(SD_CMD17_READ_SINGLE_BLOCK, s_block_address(sector), buffer, SD_BLOCK_SIZE) != STATUS_CODE_OK) {
      return RES_ERROR;
    }
    buffer += SD_BLOCK_SIZE;
    ++sector;
  }
  return RES_OK;
}

static DRESULT sd_write_blocks(BYTE pdrv, const BYTE *buffer, LBA_t sector, UINT count) {
  if (!s_is_initialized) {
    return RES_NOTRDY;
  }
  if (buffer == NULL || count == 0U) {
    return RES_PARERR;
  }

  for (UINT i = 0U; i < count; ++i) {
    if (s_write_block(s_block_address(sector), buffer) != STATUS_CODE_OK) {
      return RES_ERROR;
    }
    buffer += SD_BLOCK_SIZE;
    ++sector;
  }
  return RES_OK;
}

static DRESULT sd_card_ioctl(BYTE pdrv, BYTE command, void *buffer) {
  if (!s_is_initialized) {
    return RES_NOTRDY;
  }
  if (command != CTRL_SYNC && buffer == NULL) {
    return RES_PARERR;
  }

  switch (command) {
    case CTRL_SYNC: {
      StatusCode status = s_select_card();
      if (status == STATUS_CODE_OK) {
        status = s_wait_for_ready();
      }
      return s_finish_transaction(status) == STATUS_CODE_OK ? RES_OK : RES_ERROR;
    }
    case GET_SECTOR_SIZE:
      *(WORD *)buffer = SD_BLOCK_SIZE;
      return RES_OK;
    case GET_BLOCK_SIZE:
      *(DWORD *)buffer = 1U;
      return RES_OK;
    case GET_SECTOR_COUNT:
      return s_read_sector_count((LBA_t *)buffer) == STATUS_CODE_OK ? RES_OK : RES_ERROR;
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
