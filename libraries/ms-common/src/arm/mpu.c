/************************************************************************************************
 * @file   mpu.c
 *
 * @brief  MPU Library Source code
 *
 * @date   2024-11-03
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "stm32l4xx_hal.h"

/* Intra-component Headers */
#include "mpu.h"

#define MPU_REGION_MIN_SIZE 32U
#define IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1U)) == 0))

static StatusCode s_validate_region_settings(MPURegion *region, MPURegionSettings *settings) {
  /* Validate region number */
  if (region->number >= NUM_MPU_REGIONS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Validate alignment */
  if (region->base_address & (region->size - 1U)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Validate region size (must be power of 2 and >= 32 bytes) */
  if (!IS_POWER_OF_TWO(region->size) || region->size < MPU_REGION_MIN_SIZE) {
    return STATUS_CODE_INVALID_ARGS;
  }

  /* Validate access permissions */
  switch (settings->access_permission) {
    case MPU_REGION_NO_ACCESS:
    case MPU_REGION_PRIV_RW:
    case MPU_REGION_PRIV_RW_URO:
    case MPU_REGION_FULL_ACCESS:
    case MPU_REGION_PRIV_RO:
    case MPU_REGION_PRIV_RO_URO:
      break;
    default:
      return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode mpu_configure_region(MPURegion *region, MPURegionSettings *settings) {
  if ((region == NULL) || (settings == NULL)) {
    return STATUS_CODE_INVALID_ARGS;
  }

  status_ok_or_return(s_validate_region_settings(region, settings));

  MPU_Region_InitTypeDef init;

  init.Enable = region->enable;
  init.Number = region->number;
  init.BaseAddress = region->base_address;
  init.Size = region->size;
  init.SubRegionDisable = 0x00U;
  init.TypeExtField = 0x00U;
  init.AccessPermission = settings->access_permission;
  init.DisableExec = settings->disable_code_exec;
  init.IsShareable = 0x00U;
  init.IsCacheable = settings->is_cacheable;
  init.IsBufferable = settings->is_bufferable;

  HAL_MPU_ConfigRegion(&init);
  mpu_enable_region(region->number);

  return STATUS_CODE_OK;
}

StatusCode mpu_enable_region(MPURegionNumber region_number) {
  if (region_number >= NUM_MPU_REGIONS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  HAL_MPU_EnableRegion(region_number);

  return STATUS_CODE_OK;
}

StatusCode mpu_disable_region(MPURegionNumber region_number) {
  if (region_number >= NUM_MPU_REGIONS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  HAL_MPU_DisableRegion(region_number);

  return STATUS_CODE_OK;
}

StatusCode mpu_init(void) {
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  return STATUS_CODE_OK;
}
