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
#include "diskio.h"
#include "ff_gen_drv.h"

/* Intra-component Headers */
#include "sd_card_interface.h"
#include "sd_card_spi.h"
#include "status.h"

#define NUM_SD_SPI_PORTS 3U

/*— saved state for the linked driver —*/
static SdSpiPort s_spi_port;
static SdSpiSettings *s_settings;
static char s_sd_path[4];

/*— FatFs callback prototypes —*/
static DSTATUS sd_disk_initialize(BYTE pdrv);
static DSTATUS sd_disk_status(BYTE pdrv);
static DRESULT sd_disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count);
static DRESULT sd_disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count);
static DRESULT sd_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);

/**
 * @brief   Diskio driver table instance.
 */
static Diskio_drvTypeDef sd_disk_driver = { .disk_initialize = sd_disk_initialize, .disk_status = sd_disk_status, .disk_read = sd_disk_read, .disk_write = sd_disk_write, .disk_ioctl = sd_disk_ioctl };

StatusCode sd_card_link_driver(SdSpiPort spi, SdSpiSettings *settings) {
  if (spi >= NUM_SD_SPI_PORTS || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_spi_port = spi;
  s_settings = settings;

  /* Initialize SPI peripheral & CS line */
  StatusCode status = sd_card_spi_init(spi, settings);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  /* Link driver to FatFs as drive "0:" */
  FATFS_LinkDriver(&sd_disk_driver, s_sd_path);
  return STATUS_CODE_OK;
}

/**
 * @brief   FatFs disk_initialize callback.
 * @param   pdrv    Physical drive number (must be 0 for SD).
 * @return  DSTATUS 0=OK, STA_NOINIT otherwise.
 */
static DSTATUS sd_disk_initialize(BYTE pdrv) {
  if (pdrv != 0) {
    return STA_NOINIT;
  }
  return (sd_card_spi_init(s_spi_port, s_settings) == STATUS_CODE_OK) ? RES_OK : STA_NOINIT;
}

/**
 * @brief   FatFs disk_status callback.
 * @param   pdrv    Physical drive number (must be 0 for SD).
 * @return  DSTATUS 0=OK, STA_NOINIT otherwise.
 */
static DSTATUS sd_disk_status(BYTE pdrv) {
  if (pdrv != 0) {
    return STA_NOINIT;
  }
  return (sd_is_initialized(s_spi_port) == STATUS_CODE_OK) ? RES_OK : STA_NOINIT;
}

/**
 * @brief   FatFs disk_read callback.
 * @param   pdrv    Physical drive number (must be 0).
 * @param   buff    Pointer to data buffer.
 * @param   sector  Logical sector to start reading.
 * @param   count   Number of sectors to read.
 * @return  DRESULT RES_OK on success, RES_PARERR for bad args,
 *                  RES_ERROR on read failure.
 */
static DRESULT sd_disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  if (pdrv != 0 || count == 0) {
    return RES_PARERR;
  }
  return (sd_read_blocks(s_spi_port, buff, sector, count) == STATUS_CODE_OK) ? RES_OK : RES_ERROR;
}

/**
 * @brief   FatFs disk_write callback.
 * @param   pdrv    Physical drive number (must be 0).
 * @param   buff    Pointer to data to write.
 * @param   sector  Logical sector to start writing.
 * @param   count   Number of sectors to write.
 * @return  DRESULT RES_OK on success, RES_PARERR for bad args,
 *                  RES_ERROR on write failure.
 */
static DRESULT sd_disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  if (pdrv != 0 || count == 0) {
    return RES_PARERR;
  }
  return (sd_write_blocks(s_spi_port, (uint8_t *)buff, sector, count) == STATUS_CODE_OK) ? RES_OK : RES_ERROR;
}

/**
 * @brief   FatFs disk_ioctl callback.
 * @param   pdrv  Physical drive number (must be 0).
 * @param   cmd   Control command code.
 * @param   buff  Pointer to data buffer for control.
 * @return  DRESULT  RES_OK on success, RES_PARERR for unsupported
 *                   or bad args.
 */
static DRESULT sd_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  if (pdrv != 0) {
    return RES_PARERR;
  }

  switch (cmd) {
    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_SIZE:
      *(WORD *)buff = SD_BLOCK_SIZE;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1;
      return RES_OK;

    case GET_SECTOR_COUNT:
      *(DWORD *)buff = 4096000UL; /**< ~2 GiB placeholder */
      return RES_OK;

    default:
      return RES_PARERR;
  }
}