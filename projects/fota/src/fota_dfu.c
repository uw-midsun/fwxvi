#include "fota_dfu.h"

#include "fota_datagram.h"
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

FotaError fota_dfu_init(PacketManager *packet_manager) {
  if (packet_manager == NULL) {
    return FOTA_ERROR_BOOTLOADER_PACKET_MANAGER_UNINITIALIZED;
  }

  data.curr_state = DFU_UNINITALIZED;
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

FotaError run_dfu_bootloader() {
  /* general logic should go like this (assuming bootloader is inited):
     1. get datagram
     2. (prev bootloader) update state based on "arbitration id"
     3. perform action based on current state (start, ready, receive, jump_app, fault)
     4. check for error and exit
   */

  FotaError err = FOTA_ERROR_BOOTLOADER_SUCCESS;

  // Get a datagram from the packet manager
  FotaDatagram *datagram = packet_manager_get_datagram(&pacman, data.expected_datagram_id, datagram);
  if (datagram == NULL) {
    return FOTA_ERROR_SUCCESS;
  }

  // update state based on datagram type
  switch (datagram->header.type) {
    case FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA:
      // Process metadata

      break;
    case FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK:

      break;

    case FOTA_DATAGRAM_TYPE_UPDATE_REQUEST:
      break;

    case FOTA_DATAGRAM_TYPE_VERIFICATION:
      break;
    default:
      break;
  }
  if (err != FOTA_ERROR_SUCCESS) {
      process_new_state(DFU_FAULT);
      return err;
  }
}


  static onStateChange() {
    if (data.curr_state == DFU_UNINITALIZED) {
      return FOTA_ERROR_BOOTLOADER_UNINITIALIZED;
    }

    switch (data.curr_state) {
      case DFU_IDLE:
        // Handle idle state
        break;

      case DFU_START:
        // Handle start state
        break;

      case DFU_DATA_READY:
        // Handle data ready state
        break;

      case DFU_DATA_RECEIVE:
        // Handle data receive state
        break;

      case DFU_JUMP_APP:
        // Handle jump to application state
        break;

      case DFU_FAULT:
        // Handle fault state
        return FOTA_ERROR_BOOTLOADER_INVALID_STATE;

      default:
        return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
    }
  }

  static FotaError process_new_state(DFUStates new_state) {
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
        switch (new_state) {
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
        switch (new_state) {
          case DFU_DATA_READY:
          case DFU_JUMP_APP:
          case DFU_START:
          case DFU_FAULT:
            SUCCEED();
          default:
            FAIL();
        }
      case (DFU_JUMP_APP):
        switch (new_state) {
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