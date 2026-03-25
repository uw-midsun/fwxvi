/************************************************************************************************
 * @file    bootloader_packet_handler.c
 *
 * @brief   Bootloader Packet Handler
 *
 * @date    2025-10-05
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader_packet_handler.h"
#include "bootloader_dfu.h"
#include "bootloader_crc32.h"

#define DATAGRAM_CHUNK 8

/** @brief Receive incoming chunk */
static BootloaderError s_receive_chunk(PacketManager *pm, BootloaderDatagram *datagram, uint8_t *flash_buffer, bool *is_page_built); 
/** @brief Reset `PacketManager` on a successful flash */
static BootloaderError s_on_flash_success(PacketManager *pm, uint8_t *flash_buffer);

BootloaderError packet_handler_init(PacketManager *pm) {
  *pm = (PacketManager) {
    .current_write_address = APP_ACTIVE_START_ADDRESS,
    .bytes_written = 0,
    .data_size = 0,
    .packet_crc32 = 0,
    .expected_sequence_number = 0,
    .buffer_index = 0,
    .target_nodes = 0,
    .first_byte_received = false,
    .first_ping_received = false
      //? set to something, maybe?
  //  .ping_type = BOOTLOADER_PING_NODES; 
  //  .ping_data_len = 0; 
  };

  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_start(PacketManager *pm, BootloaderDatagram *datagram) {
  pm->data_size = datagram->payload.start.data_len;
  pm->bytes_written = 0;
  pm->buffer_index = 0;
  // pm->error = 0; // ??? 
  pm->first_byte_received = false;
  pm->first_ping_received = false;
  pm->expected_sequence_number = 0;

  if (pm->data_size % BOOTLOADER_FLASH_WORD_SIZE != 0) {
    send_ack_datagram(NACK, BOOTLOADER_DATA_NOT_ALIGNED);
    return BOOTLOADER_DATA_NOT_ALIGNED;
  }

  send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_receive_data(PacketManager *pm, BootloaderDatagram *datagram, uint8_t *flash_buffer) {
  BootloaderError error = BOOTLOADER_ERROR_NONE; 
  bool is_page_built = false; 
  error = s_receive_chunk(pm, datagram, flash_buffer, &is_page_built);
  if (is_page_built && error == BOOTLOADER_ERROR_NONE) {
    error = bootloader_dfu_flash(pm, flash_buffer); 
    error = (error) ? error : s_on_flash_success(pm, flash_buffer); 
  }
  return error;
}

static BootloaderError s_receive_chunk(PacketManager *pm, BootloaderDatagram *datagram, uint8_t *flash_buffer, bool *is_page_built) {
  BootloaderError error = BOOTLOADER_ERROR_NONE;

  if (!pm->first_byte_received) {
    send_ack_datagram(NACK, BOOTLOADER_INTERNAL_ERR);
    return BOOTLOADER_INTERNAL_ERR;
  }
  if (pm->buffer_index >= BOOTLOADER_PAGE_BYTES) {
    send_ack_datagram(NACK, BOOTLOADER_BUFFER_OVERFLOW);
    return BOOTLOADER_BUFFER_OVERFLOW;
  }

  /* Prevent overflow */
  size_t bytes_to_copy = DATAGRAM_CHUNK;
  uint8_t remaining_page = BOOTLOADER_PAGE_BYTES - pm->buffer_index;
  if (remaining_page < DATAGRAM_CHUNK) {
    bytes_to_copy = remaining_page;
  }
  uint8_t remaining_data = pm->data_size - (pm->buffer_index + pm->bytes_written);
  if (remaining_data < DATAGRAM_CHUNK) {
    bytes_to_copy = remaining_data;
  }

  /* Add to the buffer */
  memcpy(flash_buffer + pm->buffer_index, datagram->payload.data.data, bytes_to_copy);
  pm->buffer_index += bytes_to_copy;

  /* Flash buffer if it is the size of one page */
  if (pm->buffer_index == BOOTLOADER_PAGE_BYTES || 
     (pm->bytes_written + pm->buffer_index) >= pm->data_size) {

    *is_page_built = true;
  }
  return BOOTLOADER_ERROR_NONE; 
}

static BootloaderError s_on_flash_success(PacketManager *pm, uint8_t *flash_buffer) {
  pm->bytes_written += pm->buffer_index;
  pm->current_write_address += pm->buffer_index; 
  pm->buffer_index = 0;

  memset(flash_buffer, 0, sizeof(flash_buffer));
  send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);

  if (pm->bytes_written >= pm->data_size && boot_verify_flash_memory()) {
    send_ack_datagram(NACK, BOOTLOADER_FLASH_MEMORY_VERIFY_FAILED);
    return BOOTLOADER_FLASH_ERR;
  }
  
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_wait_sequencing(PacketManager *pm, BootloaderDatagram *datagram) {
  if (pm->expected_sequence_number == datagram->payload.sequencing.sequence_num) {
    pm->packet_crc32 = datagram->payload.sequencing.crc32;
    pm->buffer_index = 0;
    if (datagram->payload.sequencing.sequence_num == 0) {
      pm->first_byte_received = true;
      pm->bytes_written = 0;
      pm->current_write_address = APP_ACTIVE_START_ADDRESS;
    }
    pm->expected_sequence_number++;
  } else {
    send_ack_datagram(NACK, BOOTLOADER_SEQUENCE_ERROR);
    return BOOTLOADER_SEQUENCE_ERROR;
  }
  return BOOTLOADER_ERROR_NONE;
}

//TODO Refactor to re-use some functions (like bootloader_receive_data)
BootloaderError bootloader_ping(PacketManager *pm, BootloaderDatagram *datagram, uint8_t *flash_buffer) {
  BootloaderError error = BOOTLOADER_ERROR_NONE;

  if (!pm->first_ping_received) {
    pm->buffer_index = 0; 

    /* Save type and datalen */
    pm->ping_type = datagram->payload.ping.req; 
    pm->data_size = datagram->payload.ping.data_len;
    pm->first_byte_received = true;
    pm->first_ping_received = true; 
    pm->bytes_written = 0;
    
    return error; 
  }

  memcpy(flash_buffer + pm->buffer_index, datagram->payload.data.data, 8);
  pm->buffer_index += DATAGRAM_CHUNK;  

  if (pm->buffer_index > BOOTLOADER_PAGE_BYTES) {
    /* If we somehow go over the buffer SEND ERROR datagram */
    send_ack_datagram(NACK, BOOTLOADER_BUFFER_OVERFLOW);
    return BOOTLOADER_BUFFER_OVERFLOW;
  }
  if (pm->bytes_written == pm->data_size) {
    uint32_t calculated_crc32 = boot_crc32_calculate((const uint32_t *)flash_buffer,
                                                    BYTES_TO_WORD(pm->buffer_index));
    if (calculated_crc32 != pm->packet_crc32) {
      send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE);
      return BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE;
    }

    error |= boot_flash_erase(BOOTLOADER_ADDR_TO_PAGE(pm->current_write_address), 1U);
    error |= boot_flash_write(pm->current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);
    error |= boot_flash_read(pm->current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);

    if (error != BOOTLOADER_ERROR_NONE) {
      send_ack_datagram(NACK, error);
      return error;
    }

    calculated_crc32 = boot_crc32_calculate((const uint32_t *)flash_buffer,
                                          BYTES_TO_WORD(pm->buffer_index));
    if (calculated_crc32 != pm->packet_crc32) {
      send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_AFTER_WRITE);
      return BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE;
    }
    // when done, flash, datagram->payload.ping.req = false
    //  pm->bytes_written += pm->buffer_index;
    //  pm->current_address += pm->buffer_index;
    pm->buffer_index = 0;
    datagram->payload.ping.req = false;
    memset(flash_buffer, 0, sizeof(flash_buffer));
    send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);
  }
  return error; 
}
