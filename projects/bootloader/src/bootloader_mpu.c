/************************************************************************************************
 * @file   bootloader_mpu.c
 *
 * @brief  Source file for the Memory protection unit for the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_mpu.h"

void protect_bootloader_memory(uint32_t bootloader_start_address, uint32_t bootloader_size) {
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  MPU_Region_InitTypeDef init;

  init.Enable = MPU_REGION_ENABLE;
  init.Number = MPU_REGION_NUMBER0;
  init.BaseAddress = bootloader_start_address;
  init.Size = bootloader_size;
  init.SubRegionDisable = 0x00U;
  init.TypeExtField = MPU_TEX_LEVEL0;
  init.AccessPermission = MPU_REGION_PRIV_RO_URO;
  init.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  init.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  init.IsCacheable = MPU_ACCESS_CACHEABLE;
  init.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&init);

  HAL_MPU_EnableRegion(MPU_REGION_NUMBER0);
}
