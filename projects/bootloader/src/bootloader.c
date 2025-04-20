/************************************************************************************************
 * @file   bootloader.c
 *
 * @brief  Source file for the bootloader
 *
 * @date   2025-01-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "bootloader.h"
#include "bootloader_can_datagram.h"
#include "bootloader_crc32.h"
#include "bootloader_flash.h"

// Store CAN traffic in 2048 byte buffer to write to flash
static uint8_t *flash_buffer;

static BootloaderDatagram datagram;
static BootloaderStateData prv_bootloader = { .state = BOOTLOADER_UNINITIALIZED, .error = BOOTLOADER_ERROR_NONE, .first_byte_received = false , .first_ping_received = false};

BootloaderError bootloader_init(void) {
  prv_bootloader.bytes_written = 0;
  prv_bootloader.data_size = 0;
  prv_bootloader.application_start = APPLICATION_START_ADDRESS;
  prv_bootloader.current_write_address = APPLICATION_START_ADDRESS;
  prv_bootloader.expected_sequence_number = 0;
  prv_bootloader.packet_crc32 = 0;
  prv_bootloader.state = BOOTLOADER_IDLE;
  prv_bootloader.target_nodes = 0;
  prv_bootloader.buffer_index = 0;

  boot_crc32_init();

  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_switch_states(const BootloaderStates new_state) {
  BootloaderError return_err = BOOTLOADER_ERROR_NONE;
  BootloaderStates current_state = prv_bootloader.state;
  if (current_state == new_state) {
    return return_err;
  }

  switch (current_state) {
    case BOOTLOADER_IDLE:
      if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_START || new_state == BOOTLOADER_FAULT || new_state == BOOTLOADER_PING) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_START:
      if (new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_WAIT_SEQUENCING || new_state == BOOTLOADER_FAULT) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_WAIT_SEQUENCING:
      // Should be able to go to all states
      prv_bootloader.state = new_state;
      break;

    case BOOTLOADER_DATA_RECEIVE:
      if (new_state == BOOTLOADER_START || new_state == BOOTLOADER_JUMP_APP || new_state == BOOTLOADER_WAIT_SEQUENCING || new_state == BOOTLOADER_FAULT) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_JUMP_APP:
      if (new_state == BOOTLOADER_FAULT) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_UNINITIALIZED:
      if (new_state == BOOTLOADER_IDLE) {
        prv_bootloader.state = new_state;
      } else {
        return_err = BOOTLOADER_INVALID_ARGS;
        prv_bootloader.state = BOOTLOADER_FAULT;
      }
      break;

    case BOOTLOADER_PING:
    if (new_state == BOOTLOADER_START || new_state == BOOTLOADER_JUMP_APP ||
        new_state == BOOTLOADER_WAIT_SEQUENCING || new_state == BOOTLOADER_FAULT ||
        new_state == BOOTLOADER_PING) {
      prv_bootloader.state = new_state;
    } else {
      return_err = BOOTLOADER_INVALID_ARGS;
      prv_bootloader.state = BOOTLOADER_FAULT;
    }
    break;

    default:
      return_err = BOOTLOADER_INVALID_ARGS;
      prv_bootloader.state = BOOTLOADER_FAULT;
      break;
  }

  return return_err;
}

static BootloaderError bootloader_handle_arbitration_id(Boot_CanMessage *msg) {
  switch (msg->id) {
    case BOOTLOADER_CAN_START_ID:
      return bootloader_switch_states(BOOTLOADER_START);
    case BOOTLOADER_CAN_SEQUENCING_ID:
      return bootloader_switch_states(BOOTLOADER_WAIT_SEQUENCING);
    case BOOTLOADER_CAN_FLASH_ID:
      return bootloader_switch_states(BOOTLOADER_DATA_RECEIVE);
    case BOOTLOADER_CAN_JUMP_APPLICATION_ID:
      return bootloader_switch_states(BOOTLOADER_JUMP_APP);
    case BOOTLOADER_CAN_PING_DATA_ID:
    return bootloader_switch_states(BOOTLOADER_PING);
    default:
      return BOOTLOADER_INVALID_ARGS;
  }
}

static BootloaderError bootloader_start() {
  prv_bootloader.data_size = datagram.payload.start.data_len;
  prv_bootloader.bytes_written = 0;
  prv_bootloader.buffer_index = 0;
  prv_bootloader.error = 0;
  prv_bootloader.first_byte_received = false;
  prv_bootloader.expected_sequence_number = 0;

  if (prv_bootloader.data_size % BOOTLOADER_FLASH_WORD_SIZE != 0) {
    send_ack_datagram(NACK, BOOTLOADER_DATA_NOT_ALIGNED);
    return BOOTLOADER_DATA_NOT_ALIGNED;
  }

  send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);
  return BOOTLOADER_ERROR_NONE;
}

BootloaderError bootloader_jump_app() {
  send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);
  __asm volatile(
      "LDR     R0, =prv_bootloader  \n"
      "LDR     R1, [R0]             \n"
      "LDR     R2, [R1, #4]         \n"
      "BX      R2                   \n");

  return BOOTLOADER_INTERNAL_ERR;
}

static BootloaderError bootloader_wait_sequencing() {
  if (prv_bootloader.expected_sequence_number == datagram.payload.sequencing.sequence_num) {
    prv_bootloader.packet_crc32 = datagram.payload.sequencing.crc32;
    prv_bootloader.buffer_index = 0;
    if (datagram.payload.sequencing.sequence_num == 0) {
      prv_bootloader.first_byte_received = true;
      prv_bootloader.bytes_written = 0;
      prv_bootloader.current_write_address = APPLICATION_START_ADDRESS;
    }
    prv_bootloader.expected_sequence_number++;
  } else {
    send_ack_datagram(NACK, BOOTLOADER_SEQUENCE_ERROR);
    return BOOTLOADER_SEQUENCE_ERROR;
  }
  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_data_receive() {
  BootloaderError error = BOOTLOADER_ERROR_NONE;
  if (!prv_bootloader.first_byte_received) {
    send_ack_datagram(NACK, BOOTLOADER_INTERNAL_ERR);
    return BOOTLOADER_INTERNAL_ERR;
  }
  if (prv_bootloader.buffer_index >= BOOTLOADER_PAGE_BYTES) {
    send_ack_datagram(NACK, BOOTLOADER_BUFFER_OVERFLOW);
    return BOOTLOADER_BUFFER_OVERFLOW;
  }

  size_t bytes_to_copy = 8;
  if (BOOTLOADER_PAGE_BYTES - prv_bootloader.buffer_index < 8) {
    bytes_to_copy = BOOTLOADER_PAGE_BYTES - prv_bootloader.buffer_index;
  }
  if (prv_bootloader.data_size - (prv_bootloader.buffer_index + prv_bootloader.bytes_written) < 8) {
    bytes_to_copy = prv_bootloader.data_size - (prv_bootloader.buffer_index + prv_bootloader.bytes_written);
  }

  memcpy(flash_buffer + prv_bootloader.buffer_index, datagram.payload.data.data, bytes_to_copy);
  prv_bootloader.buffer_index += bytes_to_copy;

  if (prv_bootloader.buffer_index == BOOTLOADER_PAGE_BYTES || (prv_bootloader.bytes_written + prv_bootloader.buffer_index) >= prv_bootloader.data_size) {
    uint32_t calculated_crc32 = boot_crc32_calculate((const uint32_t *)flash_buffer, BYTES_TO_WORD(prv_bootloader.buffer_index));
    if (calculated_crc32 != prv_bootloader.packet_crc32) {
      send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE);
      return BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE;
    }

    error |= boot_flash_erase(BOOTLOADER_ADDR_TO_PAGE(prv_bootloader.current_write_address), 1U);
    error |= boot_flash_write(prv_bootloader.current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);
    error |= boot_flash_read(prv_bootloader.current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);

    if (error != BOOTLOADER_ERROR_NONE) {
      send_ack_datagram(NACK, error);
      return error;
    }

    calculated_crc32 = boot_crc32_calculate((uint32_t *)flash_buffer, BYTES_TO_WORD(prv_bootloader.buffer_index));
    if (calculated_crc32 != prv_bootloader.packet_crc32) {
      send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_AFTER_WRITE);
      return BOOTLOADER_CRC_MISMATCH_AFTER_WRITE;
    }

    prv_bootloader.bytes_written += prv_bootloader.buffer_index;
    prv_bootloader.current_write_address += prv_bootloader.buffer_index;
    prv_bootloader.buffer_index = 0;
    memset(flash_buffer, 0, sizeof(flash_buffer));
    send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);
    if (prv_bootloader.bytes_written >= prv_bootloader.data_size) {
      if (boot_verify_flash_memory()) {
        bootloader_switch_states(BOOTLOADER_FAULT);
        send_ack_datagram(NACK, BOOTLOADER_FLASH_MEMORY_VERIFY_FAILED);
      }
      return bootloader_jump_app();
    }
  }
  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_fault() {
  /* Implement code to reset the board. */
  return BOOTLOADER_INTERNAL_ERR;
}


static BootloaderError bootloader_ping() {
  BootloaderError error = BOOTLOADER_ERROR_NONE;

  if (!prv_bootloader.first_ping_received) {
    prv_bootloader.buffer_index = 0;

    // Save type and datalen
    prv_bootloader.ping_type = datagram.payload.ping.req;
    prv_bootloader.data_size = datagram.payload.ping.data_len;
    prv_bootloader.first_byte_received = true;
    prv_bootloader.bytes_written = 0;
  } else {
    // Start handling the rest of the datagrams (ping data)
    memcpy(flash_buffer + prv_bootloader.buffer_index, datagram.payload.data.data, 8);
    prv_bootloader.buffer_index += 8;

    if (prv_bootloader.buffer_index > BOOTLOADER_PAGE_BYTES) {
      // If we somehow go over the buffer SEND ERROR datagram
      send_ack_datagram(NACK, BOOTLOADER_BUFFER_OVERFLOW);
      return BOOTLOADER_BUFFER_OVERFLOW;
    }

    if (prv_bootloader.bytes_written == prv_bootloader.data_size) {
      uint32_t calculated_crc32 = boot_crc32_calculate((const uint32_t *)flash_buffer,
                                                     BYTES_TO_WORD(prv_bootloader.buffer_index));
      if (calculated_crc32 != prv_bootloader.packet_crc32) {
        send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE);
        return BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE;
      }

      error |= boot_flash_erase(BOOTLOADER_ADDR_TO_PAGE(prv_bootloader.current_write_address), 1U);
      error |= boot_flash_write(prv_bootloader.current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);
      error |= boot_flash_read(prv_bootloader.current_write_address, flash_buffer, BOOTLOADER_PAGE_BYTES);

      if (error != BOOTLOADER_ERROR_NONE) {
        send_ack_datagram(NACK, error);
        return error;
      }

      calculated_crc32 = boot_crc32_calculate((const uint32_t *)flash_buffer,
                                            BYTES_TO_WORD(prv_bootloader.buffer_index));
      if (calculated_crc32 != prv_bootloader.packet_crc32) {
        send_ack_datagram(NACK, BOOTLOADER_CRC_MISMATCH_AFTER_WRITE);
        return BOOTLOADER_CRC_MISMATCH_BEFORE_WRITE;
      }
      // when done, flash, datagram.payload.ping.req = false
      //  prv_bootloader.bytes_written += prv_bootloader.buffer_index;
      //  prv_bootloader.current_address += prv_bootloader.buffer_index;
      prv_bootloader.buffer_index = 0;
      datagram.payload.ping.req = false;
      memset(flash_buffer, 0, sizeof(flash_buffer));
      send_ack_datagram(ACK, BOOTLOADER_ERROR_NONE);
    }
  }
  return BOOTLOADER_ERROR_NONE;
}

static BootloaderError bootloader_run_state() {
  switch (prv_bootloader.state) {
    case BOOTLOADER_UNINITIALIZED:
      return BOOTLOADER_INVALID_ARGS;
      break;
    case BOOTLOADER_IDLE:
      return BOOTLOADER_ERROR_NONE;
      break;
    case BOOTLOADER_START:
      return bootloader_start();
      break;
    case BOOTLOADER_WAIT_SEQUENCING:
      return bootloader_wait_sequencing();
    case BOOTLOADER_DATA_RECEIVE:
      return bootloader_data_receive();
    case BOOTLOADER_JUMP_APP:
      return bootloader_jump_app();
    case BOOTLOADER_FAULT:
      return bootloader_fault();
    case BOOTLOADER_PING:
      return bootloader_ping();
    default:
      return BOOTLOADER_INTERNAL_ERR;
  }
}

BootloaderError bootloader_run(Boot_CanMessage *msg) {
  BootloaderError ret = BOOTLOADER_ERROR_NONE;

  if (prv_bootloader.state == BOOTLOADER_UNINITIALIZED) {
    return BOOTLOADER_ERROR_UNINITIALIZED;
  }

  datagram = deserialize_datagram(msg, &prv_bootloader.target_nodes);

  // if (!(prv_bootloader.target_nodes & (1 << _node_id))) {
  //   // Not the target node
  //   return BOOTLOADER_ERROR_NONE;
  // }

  ret = bootloader_handle_arbitration_id(msg);
  ret = bootloader_run_state();

  if (ret != BOOTLOADER_ERROR_NONE) {
    bootloader_switch_states(BOOTLOADER_FAULT);
  }

  return ret;
}
