/*
Linker subscript for STM32L433CCU6 definitions with 256K Flash and 64K RAM
*/

/* Memory Spaces Definitions */

ENTRY(Reset_Handler)

/* highest address of the user mode stack */
_sram_start = ORIGIN(RAM);
_sram_size = LENGTH(RAM);
_estack = _sram_start + _sram_size;

_flash_start = ORIGIN(FLASH);
_flash_size = LENGTH(FLASH);
_flash_page_size = 2K;

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
  RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
}

