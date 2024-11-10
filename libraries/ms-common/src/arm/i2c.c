/************************************************************************************************
 * i2c.c
 *
 * I2C Library Source Code
 *
 * Created: 2024-11-09
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */
#include <stdbool.h>
#include <stdio.h>

/* Inter-component Headers */
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_rcc.h"

/* Intra-component Headers */
#include "i2c.h"
#include "log.h"
#include "interrupts.h"
#include "queues.h"
#include "semaphore.h"

typedef enum I2CMode {
  I2C_MODE_TRANSMIT = 0,
  I2C_MODE_RECEIVE,
  NUM_I2C_MODES,
} I2CMode;

// Used to protect I2C resources
// All data tx'd and rx'd through queue
// Transactions on each I2C port protected by Mutex
typedef struct {
  uint8_t buf[I2C_MAX_NUM_DATA];
  Queue queue;
  Semaphore wait_txn;
} I2CBuffer;

typedef struct {
  uint32_t periph;                // Clock config
  uint8_t ev_irqn;                // Event Interrupt
  uint8_t err_irqn;               // Error Interrupt
  I2C_TypeDef *base;              // Base Peripheral defined by CMSIS
  I2CSettings settings;           // I2C Config
  I2CBuffer i2c_buf;              // RTOS data structures
  bool curr_mode;                 // Mode used in current transxn
  Mutex mutex;                    // Ensure port only accessed from one task at time
  I2CAddress current_addr;        // Addr used in current transxn
  volatile uint8_t num_rx_bytes;  // Number of bytes left to receive in rx mode
  volatile StatusCode exit;       // Flag set if error occurs in txn
  bool multi_txn;                 // if we are doing more than one txn don't send stop
} I2CPortData;

static I2CPortData s_port[NUM_I2C_PORTS] = {
  [I2C_PORT_1] = { .periph = RCC_APB1Periph_I2C1,
                   .base = I2C1,
                   .ev_irqn = I2C1_EV_IRQn,
                   .err_irqn = I2C1_ER_IRQn },
  [I2C_PORT_2] = { .periph = RCC_APB1Periph_I2C2,
                   .base = I2C2,
                   .ev_irqn = I2C2_EV_IRQn,
                   .err_irqn = I2C2_ER_IRQn },
};

// Generated using the I2C timing
static const uint32_t s_i2c_timing[] = {
  [I2C_SPEED_STANDARD] = 100000,  // 100 kHz
  [I2C_SPEED_FAST] = 400000,      // 400 kHz
};

static I2C_HandleTypeDef s_i2c_handles[NUM_I2C_PORTS];

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  } else if (settings->speed >= NUM_I2C_SPEEDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C speed.");
  }

  s_port[i2c].settings = *settings;
  
  // Enable clock for I2C
  switch (s_port[i2c].periph) {
    case RCC_APB1Periph_I2C1:
        __HAL_RCC_I2C1_CLK_ENABLE();
        break;
    case RCC_APB1Periph_I2C2:
        __HAL_RCC_I2C2_CLK_ENABLE();
        break;
    default:
        break;
  }

  // Enable clock for GPIOB
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // Remap pins to I2C pins B8 & B9 on Port 1
  if (i2c == I2C_PORT_1) {
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
  }
 
  // NOTE(mitch): This shouldn't be required for I2C, was needed to get around
  // Issue with SCL not being set high
  gpio_init_pin(&(settings->scl), GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&(settings->sda), GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  // Initialize pins to correct mode to operate I2C (open drain)
  // SCL should be open drain as well, but this was not working with hardware
  // Since we only use one master shouldn't be an issue
  gpio_init_pin(&(settings->scl), GPIO_ALTFN_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&(settings->sda), GPIO_ALFTN_OPEN_DRAIN, GPIO_STATE_HIGH);

  s_i2c_handles[i2c].Instance = s_port[uart].base;
  s_i2c_handles[i2c].Init.ClockSpeed = s_i2c_timing[settings->speed];
  s_i2c_handles[i2c].Init.DutyCycle = I2C_DUTYCYCLE_2;
  s_i2c_handles[i2c].Init.OwnAddress1 = 0;
  s_i2c_handles[i2c].Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  s_i2c_handles[i2c].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  s_i2c_handles[i2c].Init.OwnAddress2 = 0;
  s_i2c_handles[i2c].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  s_i2c_handles[i2c].Init.GeneralCallMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&s_i2c_handles[i2c]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  interrupt_nvic_enable(s_port[i2c].ev_irqn, INTERRUPT_PRIORITY_NORMAL);
  interrupt_nvic_enable(s_port[i2c].err_irqn, INTERRUPT_PRIORITY_NORMAL);
  __HAL_I2C_DISABLE_IT(&s_i2c_handles[i2c], I2C_IT_ERR | I2C_IT_EVT | I2C_IT_BUF);

  // Configure data structures
  s_port[i2c].i2c_buf.queue.num_items = I2C_MAX_NUM_DATA;
  s_port[i2c].i2c_buf.queue.item_size = sizeof(uint8_t);
  s_port[i2c].i2c_buf.queue.storage_buf = s_port[i2c].i2c_buf.buf;
  s_port[i2c].exit = STATUS_CODE_OK;
  s_port[i2c].multi_txn = false;
  status_ok_or_return(sem_init(&s_port[i2c].i2c_buf.wait_txn, 1, 0));
  status_ok_or_return(mutex_init(&s_port[i2c].mutex));
  status_ok_or_return(queue_init(&s_port[i2c].i2c_buf.queue));

  // Enable I2C peripheral
  __HAL_I2C_ENABLE(&s_i2c_handles[i2c]);

  return STATUS_CODE_OK;
}