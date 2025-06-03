// sd_card_interface.h
#pragma once

#include "status.h"
#include "ff_gen_drv.h"
#include "sd_card_spi.h"

/**
 * @brief   Link the SD card driver with the FATFs API and initialize it.
 * @param   spi       which SPI port to use for the SD card
 * @param   settings  pointer to your pre-configured SdSpiSettings
 * @return  STATUS_CODE_OK on success, or an error code.
 */
StatusCode sd_card_link_driver(SdSpiPort spi, SdSpiSettings *settings);