#include "fota_datagram.h"
#include "fota_dfu.h"
#include "fota_memory_map.h"
#include "network.h"
#include "network_buffer.h"
#include "packet_manger.h"

static DFUStateData data;
static PacketManager pacman;
static UartSettings pacman_settings = {
  .tx = { .port = GPIO_PORT_A, .pin = GPIO_PIN_0 },  // placeholder settings
  .rx = { .port = GPIO_PORT_B, .pin = GPIO_PIN_2 },  // placeholder settings
  .baudrate = 1024                                   // paceholder baud rate
};


FotaError fota_dfu_init(void) {
  data.curr_state = DFU_UNINITALIZED;

  if (packet_manager_init(&pacman, &pacman_settings, NULL) != FOTA_ERROR_SUCCESS) return FOTA_ERROR_INTERNAL_ERROR;

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

FotaError fota_dfu_jump_app() {}

FotaError process_new_state(DFUStates new_state) {
// Macro on success and failure
#define SUCCEED()              \
  data.curr_state = new_state; \
  return onStateChange(new_state);

#define FAIL() return FOTA_ERROR_BOOTLOADER_INVALID_STATE;

  if (data.curr_state == DFU_UNINITALIZED) {
    return FOTA_ERROR_BOOTLOADER_UNINITIALIZED;
  }

  if (new_state == data.curr_state) return FOTA_ERROR_BOOTLOADER_SUCCESS;

  switch (data.curr_state) {
    case (DFU_UNINITALIZED):
      switch (new_state) {
        case DFU_DATA_READY:
        case DFU_IDLE:
          SUCCEED();
        default:
          FAIL();
      }

    case (DFU_IDLE):
      switch (new_state) {
        case DFU_START:
        case DFU_JUMP_APP:
        case DFU_FAULT:
          SUCCEED();
        default:
          FAIL()
      }

    case (DFU_START):
      switch (new_state)
      {
      case DFU_DATA_READY:
      case DFU_JUMP_APP:
      case DFU_FAULT:
        SUCCEED();
      default:
        FAIL();
      }

    case (DFU_DATA_READY):
      SUCCEED();

    case (DFU_DATA_RECEIVE):
      switch (new_state)
      {
      case DFU_DATA_READY:
      case DFU_JUMP_APP:
      case DFU_START:
      case DFU_FAULT:
        SUCCEED();
      default:
        FAIL();
      } 
    case (DFU_JUMP_APP):
      switch (new_state){
        case DFU_FAULT:
          SUCCEED();
        default:
          FAIL();
      }
    default:
      data.curr_state = DFU_FAULT;
      return FOTA_ERROR_INVALID_ARGS;
  }
}
