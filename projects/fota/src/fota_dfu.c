/************************************************************************************************
 * @file    fota_dfu.c
 *
 * @brief   Fota DFU Source File
 *
 * @date    2025-06-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "fota_datagram.h"
#include "fota_dfu.h"
#include "fota_error.h"
#include "fota_flash.h"
#include "fota_jump_handler.h"
#include "fota_memory_map.h"
#include "network.h"
#include "network_buffer.h"
#include "packet_manger.h"

/* Intra-component Headers */

static DFUStateData data;

static bool is_dfu_init = false;

FotaError fota_dfu_init(PacketManager *pacman) {
  if (pacman == NULL) {
    return FOTA_ERROR_BOOTLOADER_UNINITIALIZED;
  }

  data.curr_state = DFU_UNINIT;
  data.bytes_written = 0;
  data.application_start = 0;
  data.binary_size = APPLICATION_SIZE;
  data.current_address = FLASH_START_ADDRESS_LINKERSCRIPT;
  data.packet_manager = &pacman;
  data.expected_datagram_id = 0;
  is_dfu_init = true;

  return FOTA_ERROR_BOOTLOADER_SUCCESS;
}

FotaError fota_dfu_run_bootloader(FotaDatagram *dgrm) {
  FotaError err = FOTA_ERROR_BOOTLOADER_SUCCESS;

  // use the packet manager to get a datagram
  if (!is_dfu_init) err = FOTA_ERROR_BOOTLOADER_UNINITIALIZED;

  // Get a datagram from the packet manager
  if (dgrm == NULL) {
    return FOTA_ERROR_NO_DATAGRAM_FOUND;
  } else if (!dgrm->is_complete) {
    return FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM;
  }

  // update state based on datagram type
  switch (dgrm->header.type) {
    case FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA:
      switch (data.curr_state) {
        case DFU_UNINIT:
          // Initialize the DFU state with metadata
          data.binary_size = dgrm->header.total_length;
          data.curr_state = DFU_START;
          break;

        default:
          // Invalid state for receiving metadata
          err = FOTA_ERROR_BOOTLOADER_INVALID_STATE;
          break;
      }
    case FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK:
      switch (data.curr_state) {
        case DFU_START:
          data.curr_state = DFU_RX;
          err = fota_dfu_write_chunk(dgrm);
          data.curr_state = DFU_START;
          break;

        case DFU_RX:
          // Dont allow writing if we are already in the process of writing
          err = FOTA_ERROR_BOOTLOADER_WRITE_IN_PROGRESS;
          break;

        case DFU_JUMP:
          // Handle jump to application logic here
          // Invalid state since we are still receiving chunks while waiting to jump
          err = FOTA_ERROR_BOOTLOADER_INVALID_STATE;
          break;

        default:
          // Invalid state for receiving firmware chunk
          err = FOTA_ERROR_BOOTLOADER_INVALID_STATE;
          break;
      }
      break;
    default:
      err = FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM;
      break;
  }

  if (data.bytes_written >= data.binary_size) {
    // If we have reached the end of the binary size, we can jump to the application
    if (!fota_verify_flash_memory()) return FOTA_ERROR_FLASH_VERIFICATION_FAILED;

    if (fota_move_app(data.application_start, FLASH_START_ADDRESS_LINKERSCRIPT) != FOTA_ERROR_SUCCESS) {
      return FOTA_ERROR_BOOTLOADER_FAILURE;
    }

    data.curr_state = DFU_JUMP;
    fota_dfu_jump_app();
  }
}

static FotaError fota_move_app(uintptr_t application_start, uintptr_t current_address) {
  // Write firmware binary to the application start address
  FotaError err = fota_flash_write(application_start, (uint8_t *)current_address, data.binary_size);
  if (err != FOTA_ERROR_SUCCESS) {
    return err;
  }
}

static FotaError fota_dfu_write_chunk(FotaDatagram *dgrm) {
  // Write the chunk to flash memory

  if (dgrm->header.datagram_id != data.expected_datagram_id) {
    return FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM;
  }

  FotaError err = fota_flash_write(data.current_address, dgrm->data, dgrm->header.total_length);
  data.current_address += dgrm->header.total_length;
  data.bytes_written += dgrm->header.total_length;
  data.expected_datagram_id++;

  return err;
}

void fota_dfu_jump_app(void) {
  fota_jump(FOTA_JUMP_APPLICATION);
}
