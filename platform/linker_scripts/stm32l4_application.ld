/*
This is the linker script for the bootloaded application code on STM32L433CCU6.
*/

/* include the common memory spaces definitions subscript */
INCLUDE "bootloader/l4_memory.ld"

/* include the application-specific sections management subscript */
INCLUDE "bootloader/sections_app.ld"