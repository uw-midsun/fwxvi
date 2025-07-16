#pragma once

/************************************************************************************************
 * @file   bootloader_mpu.h
 *
 * @brief  Header file for the Memory protection unit for the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdint.h>

/* Inter-component Headers */
#include "stm32l4xx_hal.h"

/* Intra-component Headers */

/**
 * @defgroup bootloader
 * @brief    bootloader Firmware
 * @{
 */

/**
 * @brief   Protect the bootloader memory provided the bootloaders start address and size
 * @param   bootloader_start_address The start address defined in the linkerscript
 * @param   bootloader_size The bootloader size defined in the linkerscript
 */
void protect_bootloader_memory(uint32_t bootloader_start_address, uint32_t bootloader_size);



// scons




/** @} */
