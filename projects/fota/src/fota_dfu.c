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


FotaError process_state(DFUStates state) {
    if (data.curr_state == DFU_UNINITALIZED);

    if (state == data.curr_state)
        return FOTA_ERROR_BOOTLOADER_SUCCESS;

    switch(state) {
        case (DFU_UNINITALIZED):
            return FOTA_ERROR_BOOTLOADER_INVALID_STATE;


        case (DFU_IDLE):
            return FOTA_ERROR_BOOTLOADER_SUCCESS;

        case (DFU_START):
            
    }


}