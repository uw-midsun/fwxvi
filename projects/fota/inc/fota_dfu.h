#pragma once 

/************************************************************************************************
 * @file  fota_dfu.h
 *
 * @brief  State machine for Device Firmware Update (DFU)
 *
 * @date   2025-05-14
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */


/* Intra-component Headers */
#include "fota_flash.h"
#include "fota_datagram.h"
#include "fota_error.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * @brief  implementation of dfu state machine 
 */


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

typedef struct {

    /// @brief Start of application in memory. Flashing will start here.
    uintptr_t application_start;

    /// @brief Current address that will be flashed
    uintptr_t current_address;

    /// @brief number of bytes flashed
    uint32_t bytes_written;

    /// @brief Size of application bin being flashed
    uint32_t binary_size;

    /// @brief Validation of sequencing of packets. Checked when sequencing message is recieved
    uint16_t expected_sequence_number;

    /// @brief Increment when data message is recieved, 8 bytes at a time (message length)
    uint16_t buffer_index;

    /// @brief (TBD/ need to revise): Current State of DFU
    DFUStates states;
} DFUStateData;

/// @brief Initializes DFU
/// @return 
FotaError dfu_init(void);

/// @brief Finite State Machine for DFU
/// @param new_state next state to be processed
/// @return error code
FotaError dfu_run(DFUStates new_state);

/// @brief jumps to application layer mentioned in ld script
/// @return error code
FotaError dfu_jump_app(void); 