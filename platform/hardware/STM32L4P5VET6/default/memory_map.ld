/*
Linker script for STM32L4P5VET6 (512K Flash, 320K RAM)
Bootstrap:    32K  at 0x08000000
Bootloader:   64K  at 0x08008000
App Active:   120K at 0x08018000
App Staging:  120K at 0x0802A000
FS Storage:   176K at 0x0803C000
RAM:          320K at 0x20000000
*/

ENTRY(Reset_Handler)

MEMORY
{
  FLASH        (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
  BOOTSTRAP    (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
  BOOTLOADER   (rx)  : ORIGIN = 0x08008000, LENGTH = 64K
  APP_ACTIVE   (rx)  : ORIGIN = 0x08018000, LENGTH = 120K
  APP_STAGING  (rx)  : ORIGIN = 0x0802A000, LENGTH = 120K
  FS_STORAGE   (rx)  : ORIGIN = 0x0803C000, LENGTH = 176K
  RAM          (rwx) : ORIGIN = 0x20000000, LENGTH = 320K
}

_flash_start           = ORIGIN(FLASH);
_flash_size            = LENGTH(FLASH);
_flash_page_size       = 2K;

_bootstrap_start       = ORIGIN(BOOTSTRAP);
_bootstrap_size        = LENGTH(BOOTSTRAP);

_bootloader_start      = ORIGIN(BOOTLOADER);
_bootloader_size       = LENGTH(BOOTLOADER);

_app_active_start      = ORIGIN(APP_ACTIVE);
_app_active_size       = LENGTH(APP_ACTIVE);

_app_staging_start     = ORIGIN(APP_STAGING);
_app_staging_size      = LENGTH(APP_STAGING);

_fs_storage_start      = ORIGIN(FS_STORAGE);
_fs_storage_size       = LENGTH(FS_STORAGE);

/* RAM info */
_vector_table_size     = 0x14C;
_sram_start            = ORIGIN(RAM);
_sram_size             = LENGTH(RAM);
_estack                = _sram_start + _sram_size;
