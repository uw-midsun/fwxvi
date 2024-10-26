/*
This is the linker script for the bootloader on STM32L433CCU6.
*/

/* include the common memory spaces definitions subscript */
INCLUDE "bootloader/l4_memory.ld"

/* include the bootloader-specific sections management subscript */
INCLUDE "bootloader/sections_bootloader.ld"