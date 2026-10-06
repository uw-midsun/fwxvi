#pragma once

/************************************************************************************************
 * @file    sd_card_interface.h
 *
 * @brief   Sd Card Interface
 *
 * @date    2025-06-07
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "ff.h"
#include "ff_gen_drv.h"

/* Intra-component Headers */
#include "sd_card_spi.h"
#include "status.h"

/**
 * @defgroup telemetry
 * @brief    telemetry Firmware
 * @{
 */

/**
 * @brief   Register the SD card driver with FatFs and store its SPI settings.
 * @param   spi       which SPI port to use for the SD card
 * @param   settings  pointer to your pre-configured SdSpiSettings
 * @return  STATUS_CODE_OK on success, or an error code.
 */
StatusCode sd_card_link_driver(SdSpiPort spi, SdSpiSettings *settings);

/**
 * @brief Initialize the linked SD card and mount its filesystem immediately.
 * @return FR_OK when the filesystem is mounted, otherwise a FatFs error.
 */
FRESULT sd_card_mount(void);

/** @brief Return the persistent drive path, or NULL before successful registration. */
const char *sd_card_drive_path(void);

/** @} */
