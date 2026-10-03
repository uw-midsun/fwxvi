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

UartSettings log_uart_settings = { .tx = { .port = LOG_GPIO_PORT, .pin = LOG_TX_PIN }, .rx = { .port = LOG_GPIO_PORT, .pin = LOG_RX_PIN }, .baudrate = 115200 };

static uint8_t logger_queue_buf[LOGGER_ITEMS * sizeof(LogMessageData)];

Queue logger_queue = {
  .num_items = LOGGER_ITEMS,
  .item_size = sizeof(LogMessageData),
  .storage_buf = logger_queue_buf,
};

TASK(LoggerTask, TASK_STACK_512) {
  while (true) {
    LogMessageData log_data;
    if (queue_receive(&logger_queue, &log_data, portMAX_DELAY) == STATUS_CODE_OK) {
      uart_tx(LOG_UART_PORT, (uint8_t *)log_data.log_msg, log_data.msg_size);
    }
  }
}

StatusCode log_init(void) {
  uart_init(LOG_UART_PORT, &log_uart_settings);

  queue_init(&logger_queue);
  tasks_init();

  tasks_init_task(LoggerTask, TASK_PRIORITY(TASK_PRIORTIY_LOGGER), NULL);

  return STATUS_CODE_OK;
}
