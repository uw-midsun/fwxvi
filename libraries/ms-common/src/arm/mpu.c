/************************************************************************************************
 * mpu.c
 *
 * MPU Library Source code
 *
 * Created: 2024-11-03
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */
#include "stm32l4xx_hal.h"

/* Intra-component Headers */
#include "mpu.h"

StatusCode mpu_configure_region(MPURegion *region, MPURegionSettings *settings) {
  if ((region == NULL) || (settings == NULL)) return STATUS_CODE_INVALID_ARGS;

  MPU_Region_InitTypeDef init;

  init.Enable = region->enable;
  init.Number = region->number;
  init.BaseAddress = region->base_address;
  init.Size = region->size;
  init.SubRegionDisable = 0x00;
  init.TypeExtField = 0x00;
  init.AccessPermission = settings->access_permission;
  init.DisableExec = settings->disable_exec;
  init.IsShareable = 0x00;
  init.IsCacheable = settings->is_cacheable;
  init.IsBufferable = settings->is_bufferable;

  HAL_MPU_ConfigRegion(&init);
  mpu_enable_region(region->number);

  return STATUS_CODE_OK;
}

StatusCode mpu_enable_region(uint32_t region_number) {
  HAL_MPU_EnableRegion(region_number);

  return STATUS_CODE_OK;
}

StatusCode mpu_disable_region(uint32_t region_number) {
  HAL_MPU_DisableRegion(region_number);

  return STATUS_CODE_OK;
}

StatusCode mpu_init(void) {
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  return STATUS_CODE_OK;
}
