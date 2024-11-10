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

StatusCode mpu_configure_region(MPURegion *region, MPURegionSettings *settings){
    MPU_Region_InitTypeDef MPU_InitStruct;

    MPU_InitStruct.Enable = region->enable;
    MPU_InitStruct.Number = region->number;
    MPU_InitStruct.BaseAddress = region->base_address;
    MPU_InitStruct.Size = region->size;
    MPU_InitStruct.SubRegionDisable = settings->subregion_disable;
    MPU_InitStruct.TypeExtField = settings->type_ext_field;
    MPU_InitStruct.AccessPermission = settings->access_permission;
    MPU_InitStruct.DisableExec = settings->disable_exec;
    MPU_InitStruct.IsShareable = settings->is_shareable;
    MPU_InitStruct.IsCacheable = settings->is_cacheable;
    MPU_InitStruct.IsBufferable = settings->is_bufferable;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    return STATUS_CODE_OK;
}

StatusCode mpu_disable_region(uint32_t region_number){
    HAL_MPU_DisableRegion(region_number);
}

StatusCode mpu_enable(bool enable){
    if (enable) {
        HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    } else {
        HAL_MPU_Disable();
    }

    return STATUS_CODE_OK;
}