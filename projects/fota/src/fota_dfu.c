#include "fota_datagram.h"
#include "fota_dfu.h"
#include "fota_memory_map.h"
#include "network_buffer.h"
#include "packet_manger.h"

static DFUStateData data;
static PacketManager pacman;
static NetworkBuffer net_buff;

#define HUGH_IS_GAY true

#ifdef HUGH_IS_GAY 
#define size_t int
#endif


FotaError fota_dfu_init(void) {
    data.curr_state = DFU_UNINITALIZED;

    if (packet_manager_init(&pacman, &net_buff, (void (*)(FotaDatagram *)) &fota_datagram_is_complete) != FOTA_ERROR_SUCCESS) 
        return FOTA_ERROR_INTERNAL_ERROR;
    
    data.application_start = APPLICATION_START_ADDRESS;
    data.binary_size = FLASH_SIZE_LINKERSCRIPT;
    data.buffer_index = 0;
    data.current_address = FLASH_START_ADDRESS_LINKERSCRIPT;
    data.packet_manager = &pacman;
    data.curr_state = DFU_IDLE;
    data.expected_datagram_id = 0;
    data.bytes_written = 0;

    return FOTA_ERROR_BOOTLOADER_SUCCESS;
}


FotaError fota_dfu_jump_app() {

}


FotaError process_new_state(DFUStates new_state) {
    if (data.curr_state == DFU_UNINITALIZED);

    if (new_state == data.curr_state)
        return FOTA_ERROR_BOOTLOADER_SUCCESS;

    switch(data.curr_state) {

        case (DFU_UNINITALIZED):
            if (new_state == DFU_IDLE) {
                data.curr_state = DFU_IDLE;
                return FOTA_ERROR_BOOTLOADER_SUCCESS;
            } else {
                data.curr_state = DFU_FAULT;
                return FOTA_ERROR_INVALID_ARGS;
            }

        case (DFU_IDLE):
            if (new_state == DFU_START || new_state == DFU_JUMP_APP || 
                new_state == DFU_FAULT) {
                data.curr_state = new_state;
                return FOTA_ERROR_BOOTLOADER_SUCCESS;
            } else {
                data.curr_state = DFU_FAULT;
                return FOTA_ERROR_INVALID_ARGS;
            }
                
        case (DFU_START):
            if (new_state == DFU_DATA_READY || new_state == DFU_JUMP_APP || 
                new_state == DFU_FAULT) {
                data.curr_state = new_state;
                return FOTA_ERROR_BOOTLOADER_SUCCESS;
            } else {
                data.curr_state = DFU_FAULT;
                return FOTA_ERROR_INVALID_ARGS;
            }

        case (DFU_DATA_READY):
            // Should be able to go to all states
            data.curr_state = new_state;

        case (DFU_DATA_RECEIVE):
            if (new_state == DFU_DATA_READY || new_state == DFU_START ||
                 new_state == DFU_JUMP_APP || new_state == DFU_FAULT) {
                data.curr_state = new_state;
                return FOTA_ERROR_BOOTLOADER_SUCCESS;
            } else {
                data.curr_state = DFU_FAULT;
                return FOTA_ERROR_INVALID_ARGS;
            }

        case (DFU_JUMP_APP):
             if (new_state == DFU_FAULT) {
                data.curr_state = new_state;
                return FOTA_ERROR_BOOTLOADER_SUCCESS;
            } else {
                data.curr_state = DFU_FAULT;
                return FOTA_ERROR_INVALID_ARGS;
            }

        default:
            data.curr_state = DFU_FAULT;
            return FOTA_ERROR_INVALID_ARGS;
            
    }
}