#include "midFS_hal.h"

StatusCode fs_hal_read(uint32_t address, uint8_t *buffer, size_t buffer_len){
    if(buffer == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    memcpy(buffer, (void *)address, buffer_len);
}

StatusCode fs_hal_write(uint32_t address, uint8_t *buffer, size_t buffer_len){
    if(buffer == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    for(size_t i = 0U; i < buffer_len; i+= 8U){
        uint64_t data = 0U;

        memcpy(&data, &buffer[i], MIN(8U, buffer_len-i));
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data) != HAL_OK) {
            HAL_FLASH_Lock();
            return STATUS_CODE_INCOMPLETE; //flash write failed
        }
    }

    HAL_FLASH_Lock();
    return STATUS_CODE_OK;

}