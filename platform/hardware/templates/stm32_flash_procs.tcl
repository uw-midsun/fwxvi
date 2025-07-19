# This file can be used to automatically program the STM32L4 board's Flash memory from the command line
# After programming, the board must be power-cycled (briefly remove the power) for the program to start running
# This can be done by unpluggin the USB cable, or removing JP2

init

proc stm_flash_legacy {IMGFILE} {
	reset halt
	sleep 100
	wait_halt 2

    set FLASH_START $::FLASH_START

	flash write_image erase $IMGFILE FLASH_START
	sleep 100

	verify_image $IMGFILE FLASH_START
	sleep 100
	reset run
}

proc stm_flash_bootstrap {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set BOOTSTRAP_START $::FLASH_START
    set BOOTSTRAP_END [expr {$BOOTSTRAP_START + $::BOOTSTRAP_SIZE}]
    set END_SECTOR [expr {($::BOOTSTRAP_SIZE / 1024) - 1}]

    flash erase_sector 0 0 $END_SECTOR
    sleep 100

    flash write_image $IMGFILE $BOOTSTRAP_START
    sleep 100

    verify_image $IMGFILE $BOOTSTRAP_START
    sleep 100
    reset run
}

proc stm_flash_bootloader {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set BOOTLOADER_START [expr {$::FLASH_START + $::BOOTSTRAP_SIZE}]
    set START_SECTOR [expr {$::BOOTSTRAP_SIZE / 1024}]
    set END_SECTOR [expr {($::BOOTSTRAP_SIZE + $::BOOTLOADER_SIZE)/1024 - 1}]

    flash erase_sector 0 $START_SECTOR $END_SECTOR
    sleep 100

    flash write_image $IMGFILE $BOOTLOADER_START
    sleep 100

    verify_image $IMGFILE $BOOTLOADER_START
    sleep 100
    reset run
}

proc stm_flash_app_active {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set APP_ACTIVE_START [expr {
        $::FLASH_START +
        $::BOOTSTRAP_SIZE +
        $::BOOTLOADER_SIZE +
        $::APP_ACTIVE_SIZE
    }]

    set START_SECTOR [expr {
        ($::BOOTSTRAP_SIZE + $::BOOTLOADER_SIZE + $::APP_ACTIVE_SIZE) / 1024
    }]

    set END_SECTOR [expr {
        ($::FLASH_END - $APP_ACTIVE_START) / 1024 +
        $START_SECTOR - 1
    }]

    flash erase_sector 0 $START_SECTOR $END_SECTOR
    sleep 100

    flash write_image $IMGFILE $APP_ACTIVE_START
    sleep 100

    verify_image $IMGFILE $APP_ACTIVE_START
    sleep 100
    reset run
}

proc stm_flash_app_staging {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set APP_STAGING_START [expr {
        $::FLASH_START +
        $::BOOTSTRAP_SIZE +
        $::BOOTLOADER_SIZE +
        $::APP_ACTIVE_SIZE +
        $::APP_STAGING_SIZE
    }]

    set START_SECTOR [expr {
        ($::BOOTSTRAP_SIZE + $::BOOTLOADER_SIZE + $::APP_ACTIVE_SIZE + $::APP_STAGING_SIZE) / 1024
    }]

    set END_SECTOR [expr {
        ($::FLASH_END - $APP_STAGING_START) / 1024 +
        $START_SECTOR - 1
    }]

    flash erase_sector 0 $START_SECTOR $END_SECTOR
    sleep 100

    flash write_image $IMGFILE $APP_STAGING_START
    sleep 100

    verify_image $IMGFILE $APP_STAGING_START
    sleep 100
    reset run
}

proc stm_flash_fs_storage {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set FS_STORAGE_START [expr {
        $::FLASH_START +
        $::BOOTSTRAP_SIZE +
        $::BOOTLOADER_SIZE +
        $::APP_ACTIVE_SIZE +
        $::APP_STAGING_SIZE +
        $::FS_STORAGE_SIZE
    }]

    set START_SECTOR [expr {
        ($::BOOTSTRAP_SIZE + $::BOOTLOADER_SIZE + $::APP_ACTIVE_SIZE + $::APP_STAGING_SIZE + $::FS_STORAGE_SIZE) / 1024
    }]

    set END_SECTOR [expr {
        ($::FLASH_END - $FS_STORAGE_START) / 1024 +
        $START_SECTOR - 1
    }]

    flash erase_sector 0 $START_SECTOR $END_SECTOR
    sleep 100

    flash write_image $IMGFILE $FS_STORAGE_START
    sleep 100

    verify_image $IMGFILE $FS_STORAGE_START
    sleep 100
    reset run
}

proc stm_erase {} {
	reset halt
	sleep 100
	stm32l4x mass_erase 0
	sleep 100
}
