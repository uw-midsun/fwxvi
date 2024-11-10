#pragma once

/************************************************************************************************
 * mpu.h
 *
 * MPU Library Header file
 *
 * Created: 2024-11-03
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdbool.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "status.h"

typedef struct {
    uint8_t enable;        
    uint8_t number;
    uint32_t base_address;
    uint8_t size;              
} MPURegion;

typedef struct {
    uint8_t subregion_disable;
    uint8_t type_ext_field;
    uint8_t access_permission;
    uint8_t disable_exec;
    uint8_t is_shareable; 
    uint8_t is_cacheable;
    uint8_t is_bufferable;
} MPURegionSettings;

/**
 * @brief   Configures MPU region with given settings
 * @param   region Pointer to which MPU region to configure
 * @param   settings Pointer to MPU region configuration settings
 */
StatusCode mpu_configure_region(MPURegion *region, MPURegionSettings *settings);

/**
 * @brief   Disables Given Region.
 * @param   region_number Number of MPU region to disable
 */
StatusCode mpu_disable_region(uint32_t region_number);

/**
 * @brief   Enables or Disables MPU with default memory access
 * @param   enable MPU Status
 */
StatusCode mpu_enable(bool enable);