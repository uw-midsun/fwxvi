#pragma once 

#include "fota_flash.h"
#include "fota_datagram.h"
#include "fota_error.h"

typedef enum {
    /// @brief DFU starts uninitalized
    DFU_UNINITALIZED = 0,

    /// @brief DFU is initialized and waiting for messages to flash
    DFU_IDLE,

    /// @brief DFU processing start msg 
    DFU_START,

    /// @brief DFU waiting for first byte to be sent 
    DFU_DATA_READY,

    /// @brief DFU recieving data and flashing immediately 
    DFU_DATA_RECEIVE,

    /// @brief DFU prompted to jump to application defined by APP_START_ADDRESS
    DFU_JUMP_APP,

    /// @brief bootloader is in fault state
    DFU_FAULT
} DFUStates;










