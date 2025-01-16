/*
Linker subscript for STM32L433CCU6 definitions with 256K Flash and 64K RAM
*/

ENTRY(Reset_Handler)

_bootloader_size = 64K;

_flash_start = 0x08000000;
_flash_size = 64K;
_flash_page_size = 2K;

_bootloader_start = _flash_start;
_application_start = _bootloader_start + _bootloader_size;

_application_size = _flash_size - (_application_start - _flash_start);

_vector_table_size = 0x14C; /* used for relocating the vector table to RAM for the application */

/* highest address of the user mode stack */
_sram_start = ORIGIN(RAM);
_sram_size = LENGTH(RAM);
_estack = _sram_start + _sram_size;

MEMORY
{
  BOOTLOADER  (rx) : ORIGIN = _bootloader_start, LENGTH = _bootloader_size
  APPLICATION (rx) : ORIGIN = _application_start, LENGTH = _application_size
  RAM        (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
