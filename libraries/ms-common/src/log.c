/************************************************************************************************
 * @file   log.c
 *
 * @brief  Source code for the logging library used to debug all modules
 *
 * @date   2024-11-02
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "log.h"

// Allocating memory for extern variables in .h files
char g_log_buffer[MAX_LOG_SIZE];

UartSettings log_uart_settings = { .tx = { .port = LOG_GPIO_PORT, .pin = LOG_TX_PIN }, .rx = { .port = LOG_GPIO_PORT, .pin = LOG_RX_PIN }, .baudrate = 115200 };

// make a struct -> hold sizes and messages
// change the sizes of things -> MAX_LOG_SIZE,

#define NUM_ITEMS_LOGGER 15

static uint8_t s_logger_queue_buf[NUM_ITEMS_LOGGER * sizeof(logger_message_data)];

Queue s_logger_queue = {
  .num_items = NUM_ITEMS_LOGGER,
  .item_size = sizeof(logger_message_data),
  .storage_buf = s_logger_queue_buf,
};

TASK(LoggerTask, TASK_STACK_512) {
  while (true) {
    logger_message_data log_data;
    if (queue_receive(&s_logger_queue, &log_data, 1000) == STATUS_CODE_OK) {
      uart_tx(LOG_UART_PORT, (uint8_t *)log_data.log_msg, log_data.msg_size);
    }
  }
}

StatusCode log_init(void) {
  uart_init(LOG_UART_PORT, &log_uart_settings);

  queue_init(&s_logger_queue);
  tasks_init();

  tasks_init_task(LoggerTask, TASK_PRIORITY(3U), NULL);

  return STATUS_CODE_OK;
}
