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

/* Intra-component Headers */
#include "fota_dfu.h"
#include "fota_datagram.h"
#include "fota_error.h"
#include "fota_flash.h"
#include "fota_jump_handler.h"
#include "fota_memory_map.h"
#include "network.h"
#include "network_buffer.h"

static FotaDFUContext fota_dfu_context = {
  .state = FOTA_DFU_UNINITIALIZED,
  .is_initialized = false
};

// static FotaError fota_move_app(void) {
//   if (!fota_dfu_context.is_initialized || fota_dfu_context.binary_size == 0) {
//     return FOTA_ERROR_BOOTLOADER_UNINITIALIZED;
//   }

//   if (fota_dfu_context.app_start_addr < FLASH_START_ADDRESS_LINKERSCRIPT || fota_dfu_context.app_start_addr >= FLASH_START_ADDRESS_LINKERSCRIPT + FLASH_SIZE_LINKERSCRIPT) {
//     return FOTA_ERROR_INVALID_ARGS;
//   }

//   if (fota_dfu_context.flash_base_addr < FLASH_START_ADDRESS_LINKERSCRIPT || fota_dfu_context.flash_base_addr >= FLASH_START_ADDRESS_LINKERSCRIPT + FLASH_SIZE_LINKERSCRIPT) {
//     return FOTA_ERROR_INVALID_ARGS;
//   }

//   if (fota_dfu_context.binary_size >= APPLICATION_SIZE) {
//     return FOTA_ERROR_INVALID_ARGS;
//   }

//   return fota_flash_write(fota_dfu_context.app_start_addr, (uint8_t *)fota_dfu_context.flash_base_addr, fota_dfu_context.binary_size);
// }

static FotaError fota_dfu_write_chunk(FotaDatagram *dgrm) {
  if (dgrm->header.datagram_id != fota_dfu_context.expected_datagram_id) {
    return FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM;
  }

  FotaError err = fota_flash_write(fota_dfu_context.current_write_addr, dgrm->data, dgrm->header.total_length);

  if (err != FOTA_ERROR_SUCCESS) {
    return err;
  }

  fota_dfu_context.current_write_addr += dgrm->header.total_length;
  fota_dfu_context.bytes_written += dgrm->header.total_length;
  fota_dfu_context.expected_datagram_id++;

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_dfu_init(PacketManager *packet_manager, uintptr_t staging_base, uintptr_t app_start_addr) {
  if (packet_manager == NULL) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (staging_base < FLASH_START_ADDRESS_LINKERSCRIPT || staging_base >= FLASH_START_ADDRESS_LINKERSCRIPT + FLASH_SIZE_LINKERSCRIPT) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  if (app_start_addr < FLASH_START_ADDRESS_LINKERSCRIPT || app_start_addr >= FLASH_START_ADDRESS_LINKERSCRIPT + FLASH_SIZE_LINKERSCRIPT) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  fota_dfu_context.packet_manager = packet_manager;
  fota_dfu_context.flash_base_addr = staging_base;
  fota_dfu_context.app_start_addr = app_start_addr;
  fota_dfu_context.current_write_addr = staging_base;
  fota_dfu_context.bytes_written = 0U;
  fota_dfu_context.binary_size = 0U;
  fota_dfu_context.expected_datagram_id = 0U;
  fota_dfu_context.state = FOTA_DFU_IDLE;
  fota_dfu_context.is_initialized = true;

  return FOTA_ERROR_SUCCESS;
}

FotaError fota_dfu_process(FotaDatagram *datagram) {
  if (!fota_dfu_context.is_initialized) {
    return FOTA_ERROR_BOOTLOADER_UNINITIALIZED;
  }

  if (datagram == NULL || !datagram->is_complete) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  FotaError err = FOTA_ERROR_SUCCESS;

  switch (datagram->header.type) {
    case FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA:
      if (fota_dfu_context.state != FOTA_DFU_IDLE) {
        return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
      }

      fota_dfu_context.binary_size = datagram->header.total_length;
      fota_dfu_context.current_write_addr = fota_dfu_context.flash_base_addr;
      fota_dfu_context.bytes_written = 0;
      fota_dfu_context.expected_datagram_id = 0;
      fota_dfu_context.state = FOTA_DFU_RECEIVING;
      break;

    case FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK:
      if (fota_dfu_context.state != FOTA_DFU_RECEIVING) {
        return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
      }

      err = fota_dfu_write_chunk(datagram);
      if (err != FOTA_ERROR_SUCCESS) return err;

      if (fota_dfu_context.bytes_written >= fota_dfu_context.binary_size) {
        fota_dfu_context.state = FOTA_DFU_COMPLETE;
      }
      break;

    default:
      err = FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM;
      break;
  }

  return err;
}

FotaError fota_dfu_finalize() {
  return FOTA_ERROR_SUCCESS;
}
