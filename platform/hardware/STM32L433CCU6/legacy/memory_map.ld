/*
Linker script for STM32L433CCU6 (256K Flash, 64K RAM)
Bootstrap:    32K  at 0x08000000
Bootloader:     0  at 0x08000000
App Active:     0  at 0x08000000
App Staging:  80K  at 0x08000000
FS Storage:     0  at 0x08000000
RAM:          64K  at 0x20000000
*/

ENTRY(Reset_Handler)

MEMORY
{
  FLASH        (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
  BOOTSTRAP    (rx)  : ORIGIN = 0x08000000, LENGTH = 0
  BOOTLOADER   (rx)  : ORIGIN = 0x08000000, LENGTH = 0
  APP_ACTIVE   (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
  APP_STAGING  (rx)  : ORIGIN = 0x08000000, LENGTH = 0
  FS_STORAGE   (rx)  : ORIGIN = 0x08000000, LENGTH = 0
  RAM          (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
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

