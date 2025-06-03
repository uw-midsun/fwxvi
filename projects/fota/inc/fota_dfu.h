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
#include "packet_manger.h"

/**
 * @defgroup FOTA
 * @brief    Firmware Over the Air (FOTA) Update system
 * @{
 */

/**
 * @brief  implementation of dfu state machine 
 */


typedef enum DfuStates {
    /// @brief DFU starts uninitalized
    DFU_UNINIT,

    /// @brief DFU ready
    DFU_START,
    
    /// @brief DFU writing 
    DFU_RX,

    /// @brief DFU Jump App
    DFU_JUMP,
} DFUStates;

typedef struct {

    /// @brief Start of application in memory. 
    volatile uintptr_t application_start;

    /// @brief Current address that will be flashed
    volatile uintptr_t current_address;

    /// @brief number of bytes flashed
    uint32_t bytes_written;

    /// @brief Size of application bin being flashed
    uint32_t binary_size;

    /// @brief Validation of sequencing of packets. Checked when sequencing message is recieved. Incremented by 1 for each DFU datagram processed.
    uint16_t expected_datagram_id;

    /// @brief State of the DFU state machine
    DFUStates curr_state;

    /// @brief Packet manager context structure for interfacing with processed packets.
    PacketManager *packet_manager;
} DFUStateData;

/// @brief Initializes DFU
/// @return 
FotaError fota_dfu_init(void);

/// @brief Run Finite State Machine for DFU
/// @return error code
FotaError fota_dfu_run(void);

/// @brief jumps to application layer mentioned in ld script
void fota_dfu_jump_app(void); 