#include "fota_timeout.h"

bool fota_is_timed_out(void){
    return HAL_GetTick() >= BOOTLOADER_TIMEOUT_MS;
}