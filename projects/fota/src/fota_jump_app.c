#include "fota_jump_app.h"

FotaError bootloader_jump_app(void) {
  __asm volatile(
    "LDR     R0, =_application_start  \n"
    "MSR     MSP, R0              \n"
    "LDR     R1, [R0, #4]         \n"
    "BX      R1                   \n");
    return FOTA_ERROR_JUMP_FAILED;
}