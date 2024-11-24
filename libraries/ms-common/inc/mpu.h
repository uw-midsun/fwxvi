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

typedef enum {
  MPU_REGION_0,
  MPU_REGION_1,
  MPU_REGION_2,
  MPU_REGION_3,
  MPU_REGION_4,
  MPU_REGION_5,
  MPU_REGION_6,
  MPU_REGION_7
} MPURegionNumber;

typedef enum {
  MPU_ACCESS_NONE,
  MPU_ACCESS_PRIV_RW,
  MPU_ACCESS_PRIV_RW_URO,
  MPU_ACCESS_FULL,
  MPU_ACCESS_PRIV_RO = 0x05,
  MPU_ACCESS_PRIV_RO_URO,
} MPUAccessPerm;

typedef struct {
  uint8_t enable;
  uint8_t number;
  uint32_t base_address;
  uint8_t size;
} MPURegion;

typedef struct {
  uint8_t access_permission;
  uint8_t disable_exec;
  uint8_t is_cacheable;
  uint8_t is_bufferable;
} MPURegionSettings;

/**
 * @brief   Configures MPU region with given settings
 * @param   region Pointer to which MPU region to configure
 * @param   settings Pointer to MPU region configuration settings
 * @return  STATUS_CODE_OK if MPU initialization succeeded
 * @return  STATUS_CODE_INVALID_ARGS if one of the parameters are incorrect
 */
StatusCode mpu_configure_region(MPURegion *region, MPURegionSettings *settings);

/**
 * @brief   Enables given region
 * @param   region_number Number of MPU region to enable
 * @return  STATUS_CODE_OK if mpu region was successfully enabled
 */
StatusCode mpu_enable_region(uint32_t region_number);

/**
 * @brief   Disables given region
 * @param   region_number Number of MPU region to disable
 * @return  STATUS_CODE_OK if mpu region was successfully disabled
 */
StatusCode mpu_disable_region(uint32_t region_number);

/**
 * @brief   Enables or Disables MPU with default memory access
 * @return  STATUS_CODE_OK if MPU initialization succeeded
 */
StatusCode mpu_init(void);
