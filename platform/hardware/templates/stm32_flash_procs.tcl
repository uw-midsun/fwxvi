# STM32L4 OpenOCD Flash Script
# Flashes various firmware regions using OpenOCD + ST-Link
# Automatically erases, writes, and verifies

# Required global variables to define:
# set ::FLASH_START
# set ::FLASH_END
# set ::BOOTSTRAP_SIZE
# set ::BOOTLOADER_SIZE
# set ::APP_ACTIVE_SIZE
# set ::APP_STAGING_SIZE

init

proc stm_flash_legacy {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    flash write_image erase $IMGFILE $::FLASH_START
    verify_image $IMGFILE $::FLASH_START

    reset run
}

proc stm_flash_bootstrap {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set START_ADDR $::FLASH_START

    flash write_image erase $IMGFILE $START_ADDR
    verify_image $IMGFILE $START_ADDR

    reset run
}

proc stm_flash_bootloader {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set START_ADDR [expr {$::FLASH_START + $::BOOTSTRAP_SIZE}]

    flash write_image erase $IMGFILE $START_ADDR
    verify_image $IMGFILE $START_ADDR

    reset run
}

proc stm_flash_app_active {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set START_ADDR [expr {
        $::FLASH_START +
        $::BOOTSTRAP_SIZE +
        $::BOOTLOADER_SIZE
    }]

    flash write_image erase $IMGFILE $START_ADDR
    verify_image $IMGFILE $START_ADDR

    reset run
}

proc stm_flash_fs_storage {IMGFILE} {
    reset halt
    sleep 100
    wait_halt 2

    set START_ADDR [expr {
        $::FLASH_START +
        $::BOOTSTRAP_SIZE +
        $::BOOTLOADER_SIZE +
        $::APP_ACTIVE_SIZE +
        $::APP_STAGING_SIZE
    }]

    flash write_image erase $IMGFILE $START_ADDR
    verify_image $IMGFILE $START_ADDR

    reset run
}

proc stm_erase {} {
    reset halt
    sleep 100
    stm32l4x mass_erase 0
    sleep 100
}
