/* AUTO GENERATED from board.toml, do not edit */
ENTRY(Reset_Handler)

MEMORY
{
  FLASH      (rx)  : ORIGIN = 0x08000000, LENGTH = 524288
  BOOTSTRAP  (rx)  : ORIGIN = 0x08000000, LENGTH = 8192
  BOOTLOADER (rx)  : ORIGIN = 0x08002000, LENGTH = 32768
  APP_ACTIVE (rx)  : ORIGIN = 0x0800a000, LENGTH = 475136
  CONFIG     (rw)  : ORIGIN = 0x0807e000, LENGTH = 8192
  RAM        (rwx) : ORIGIN = 0x20000000, LENGTH = 327680
}

_flash_start      = ORIGIN(FLASH);
_flash_size       = LENGTH(FLASH);
_flash_page_size  = 8192;

_bootstrap_start  = ORIGIN(BOOTSTRAP);
_bootstrap_size   = LENGTH(BOOTSTRAP);

_bootloader_start = ORIGIN(BOOTLOADER);
_bootloader_size  = LENGTH(BOOTLOADER);

_app_active_start = ORIGIN(APP_ACTIVE);
_app_active_size  = LENGTH(APP_ACTIVE);
_app_start        = _app_active_start;
_app_size         = _app_active_size;

_config_start     = ORIGIN(CONFIG);
_config_size      = LENGTH(CONFIG);

_vector_table_size = 0x14C;
_sram_start       = ORIGIN(RAM);
_sram_size        = LENGTH(RAM);
_estack           = _sram_start + _sram_size;
