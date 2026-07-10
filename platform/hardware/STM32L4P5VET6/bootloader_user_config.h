#pragma once

/************************************************************************************************
 * @file    bootloader_user_config.h
 *
 * @brief   AUTO GENERATED from board.toml, do not edit, Bootloader geometry and CAN knobs
 *
 * @date    AUTO GENERATED
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* ============================ FLASH GEOMETRY ============================ */
#define BL_FLASH_BASE         0x08000000U
#define BL_FLASH_SIZE_KB      512U
#define BL_FLASH_PAGE_SIZE_B  8192U

#define BL_BOOTSTRAP_SIZE_KB  8U
#define BL_BOOTLOADER_SIZE_KB 32U

/* =============================== SRAM ================================== */
#define BL_SRAM_BASE          0x20000000U
#define BL_SRAM_SIZE_KB       320U

/* =============================== CAN =================================== */
#define BL_CAN_BITRATE_KBPS   500U
#define BL_ENTER_ID           0x010U
#define BL_XFER_ID_BASE       0x100U
#define BL_NODE_ID            0U
