/************************************************************************************************
 * @file    fota_dfu.c
 *
 * @brief   Fota DFU Source File
 *
 * @date    2025-06-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "fota_dfu.h"
#include "fota_datagram.h"
#include "fota_datagram_payloads.h"
#include "fota_encryption.h"
#include "fota_error.h"
#include "fota_flash.h"
#include "fota_jump_handler.h"
#include "fota_memory_map.h"
#include "network.h"
#include "network_buffer.h"

#define FOTA_DFU_WORD_SIZE 4U
#define MIDNIGHT_SUN_MAGIC_NUMBER 0xDEADBEEF

static FotaDFUContext fota_dfu_context = { .state = FOTA_DFU_UNINITIALIZED, .is_initialized = false };

static FotaError fota_dfu_copy_staging_to_app(void) {
  if (!fota_dfu_context.is_initialized || fota_dfu_context.state != FOTA_DFU_COMPLETE) {
    return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
  }

  uint8_t page_buf[FOTA_PAGE_BYTES];
  uintptr_t staging_addr = fota_dfu_context.staging_base_addr;
  uintptr_t app_addr = fota_dfu_context.app_start_addr;
  uint32_t remaining_binary_size = fota_dfu_context.binary_size;

  while (remaining_binary_size) {
    uint32_t chunk = (remaining_binary_size > FOTA_PAGE_BYTES) ? FOTA_PAGE_BYTES : remaining_binary_size;

    FotaError error = fota_flash_read(staging_addr, page_buf, chunk);

    if (error != FOTA_ERROR_SUCCESS) {
      return error;
    }

    error = fota_flash_write(app_addr, page_buf, chunk);

    if (error != FOTA_ERROR_SUCCESS) {
      return error;
    }

    staging_addr += chunk;
    app_addr += chunk;
    remaining_binary_size -= chunk;
  }

  return FOTA_ERROR_SUCCESS;
}

static FotaError fota_dfu_save_application_data() {
  if (!fota_dfu_context.is_initialized || fota_dfu_context.state != FOTA_DFU_COMPLETE) {
    return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
  }

  return FOTA_ERROR_SUCCESS;
}

static FotaError fota_dfu_write_chunk(FotaDatagram *datagram) {
  if (datagram->header.type != FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK) {
    return FOTA_ERROR_INVALID_ARGS;
  }

  FotaDatagramPayload_FirmwareChunk *chunk = (FotaDatagramPayload_FirmwareChunk *)datagram->data;

  if (datagram->header.datagram_id != fota_dfu_context.expected_datagram_id) {
    return FOTA_ERROR_BOOTLOADER_SEQUENCE_OUT_OF_ORDER;
  }

  if ((fota_dfu_context.bytes_written + datagram->header.total_length) > fota_dfu_context.binary_size) {
    return FOTA_ERROR_BOOTLOADER_BINARY_OVERSIZED;
  }

  FotaError error = fota_flash_write(fota_dfu_context.current_write_addr, chunk->data, datagram->header.total_length);

  if (error != FOTA_ERROR_SUCCESS) {
    return error;
  }

  fota_dfu_context.current_write_addr += datagram->header.total_length;
  fota_dfu_context.bytes_written += datagram->header.total_length;
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
  fota_dfu_context.staging_base_addr = staging_base;
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

  switch (datagram->header.type) {
    /* FIRMWARE METADATA */
    case FOTA_DATAGRAM_TYPE_FIRMWARE_METADATA: {
      if (fota_dfu_context.state != FOTA_DFU_IDLE) {
        return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
      }

      FotaDatagramPayload_FirmwareMetadata *metadata = (FotaDatagramPayload_FirmwareMetadata *)datagram->data;

      fota_dfu_context.binary_size = metadata->binary_size;
      fota_dfu_context.expected_crc32 = metadata->expected_binary_crc32;
      fota_dfu_context.version_major = metadata->version_major;
      fota_dfu_context.version_minor = metadata->version_minor;
      memcpy(fota_dfu_context.firmware_id, metadata->firmware_id, FOTA_FIRMWARE_ID_LENGTH);
      fota_dfu_context.bytes_written = 0U;
      fota_dfu_context.expected_datagram_id = 0U;
      fota_dfu_context.current_write_addr = fota_dfu_context.staging_base_addr;

      uint32_t pages = (fota_dfu_context.binary_size + FOTA_PAGE_BYTES - 1U) / FOTA_PAGE_BYTES;
      FotaError error = fota_flash_erase(FOTA_ADDR_TO_PAGE(fota_dfu_context.staging_base_addr), pages);

      if (error != FOTA_ERROR_SUCCESS) {
        return error;
      }

      fota_dfu_context.state = FOTA_DFU_RECEIVING;
    } break;

    /* FIRMWARE CHUNK  */
    case FOTA_DATAGRAM_TYPE_FIRMWARE_CHUNK: {
      if (fota_dfu_context.state != FOTA_DFU_RECEIVING) {
        return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
      }

      FotaError error = fota_dfu_write_chunk(datagram);
      if (error != FOTA_ERROR_SUCCESS) {
        return error;
      }

      if (fota_dfu_context.bytes_written >= fota_dfu_context.binary_size) {
        fota_dfu_context.state = FOTA_DFU_COMPLETE;
      }
    } break;

    /* JUMP APPLICATIONS */
    case FOTA_DATAGRAM_TYPE_JUMP_TO_APP: {
      FotaDatagramPayload_JumpToApp *jump = (FotaDatagramPayload_JumpToApp *)datagram->data;
      if (jump->validation_flag != 1U || jump->magic_number != MIDNIGHT_SUN_MAGIC_NUMBER) {
        return FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM;
      }
      if (fota_dfu_context.state != FOTA_DFU_COMPLETE) {
        return FOTA_ERROR_BOOTLOADER_INVALID_STATE;
      }
      fota_dfu_context.state = FOTA_DFU_JUMP;
    } break;

    default:
      return FOTA_ERROR_BOOTLOADER_INVALID_DATAGRAM;
  }

  /* If we just transitioned to COMPLETE attempt final move & jump */
  if (fota_dfu_context.state == FOTA_DFU_COMPLETE) {
    /* Verify CRC32 of newly loaded application */
    uint32_t calculated_application_crc32 = fota_calculate_crc32_on_flash_memory(fota_dfu_context.staging_base_addr, fota_dfu_context.binary_size);

    if (calculated_application_crc32 != fota_dfu_context.expected_crc32) {
      return FOTA_ERROR_FLASH_VERIFICATION_FAILED;
    }

    FotaError error = fota_dfu_copy_staging_to_app();

    if (error != FOTA_ERROR_SUCCESS) {
      return error;
    }

    error = fota_dfu_save_application_data();

    fota_dfu_context.state = FOTA_DFU_JUMP;
  }

  if (fota_dfu_context.state == FOTA_DFU_JUMP) {
    FotaError error = fota_verify_flash_memory(fota_dfu_context.app_start_addr, fota_dfu_context.binary_size);

    if (error != FOTA_ERROR_SUCCESS) {
      return error;
    }

#ifndef MS_PLATFORM_X86
    fota_jump(FOTA_JUMP_APPLICATION);
    return FOTA_ERROR_BOOTLOADER_FAILURE;
#endif
  }

  return FOTA_ERROR_SUCCESS;
}
