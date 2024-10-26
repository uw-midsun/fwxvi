/*
Default linker script for STM32L433CCU6 256k flash 64k ram
Used when building non-bootloader projects normally.
*/

/* include the memory spaces definitions sub-script */
INCLUDE "default/l4_memory.ld"

/* include the sections management sub-script for FLASH mode */
INCLUDE "default/sections_default.ld"
