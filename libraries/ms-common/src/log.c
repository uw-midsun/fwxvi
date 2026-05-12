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

//have a logger funtion --> put into mcu


#define NUM_ITEMS_LOGGER 10
#define ITEM_SIZE_LOGGER 5
static uint8_t s_logger_queue_buf[NUM_ITEMS_LOGGER * ITEM_SIZE_LOGGER];

static Queue s_logger_queue = {
  .num_items = NUM_ITEMS_LOGGER,
  .item_size = ITEM_SIZE_LOGGER,
  .storage_buf = s_logger_queue_buf,
};

#define NUM_ITEMS_LOGGER_SIZE 10
#define ITEM_SIZE_LOGGER_SIZE sizeof(uint8_t)
static uint8_t s_logger_size_queue_buf[NUM_ITEMS_LOGGER_SIZE * ITEM_SIZE_LOGGER_SIZE];

static Queue s_logger_size_queue = {
  .num_items = NUM_ITEMS_LOGGER_SIZE,
  .item_size = ITEM_SIZE_LOGGER_SIZE,
  .storage_buf = s_logger_size_queue_buf,
};

TASK(LoggerTask, TASK_STACK_256){

    while (true) {
        char log_msg[MAX_LOG_SIZE];

        if(queue_receive(&s_logger_size_queue, log_msg,1000) == STATUS_CODE_OK){
            uart_tx(LOG_UART_PORT, (uint8_t *)log_msg, queue_receive(&s_logger_size_queue, log_msg, 1000));                                                                        
        }
    }
}

StatusCode log_init(void){
    uart_init(LOG_UART_PORT, &log_uart_settings);

    queue_init(&s_logger_queue);
    queue_init(&s_logger_size_queue);

    tasks_init();

    tasks_init_task(LoggerTask, TASK_PRIORITY(2U), NULL);

    return STATUS_CODE_OK;
}




