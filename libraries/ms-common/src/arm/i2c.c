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
#include "interrupts.h"
#include "log.h"
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
  [I2C_PORT_1] = { .periph = RCC_APB1ENR1_I2C1EN,
                   .base = I2C1,
                   .ev_irqn = I2C1_EV_IRQn,
                   .err_irqn = I2C1_ER_IRQn },
  [I2C_PORT_2] = { .periph = RCC_APB1ENR1_I2C2EN,
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

// Attempt to recover from a bus lockup
static StatusCode prv_recover_lockup(I2CPort i2c) {
  HAL_I2C_DeInit(&s_i2c_handles[i2c]);
  I2CSettings *settings = &s_port[i2c].settings;
  // Manually clock SCL
  gpio_init_pin(&settings->scl, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  for (size_t i = 0; i < 16; i++) {
    gpio_toggle_state(&settings->scl);
  }
  gpio_init_pin(&settings->scl, GPIO_ALTFN_PUSH_PULL, GPIO_STATE_HIGH);
  // Clear I2C error flags
  __HAL_I2C_CLEAR_FLAG(&s_i2c_handles[i2c],
                       I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR | I2C_FLAG_TIMEOUT);
  // Disable the I2C peripheral
  __HAL_I2C_DISABLE(&s_i2c_handles[i2c]);
  // Enable the I2C peripheral
  __HAL_I2C_ENABLE(&s_i2c_handles[i2c]);
  // Reinitialize
  s_i2c_handles[i2c].Instance = s_port[i2c].base;
  s_i2c_handles[i2c].Init.OwnAddress1 = 0;
  s_i2c_handles[i2c].Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  s_i2c_handles[i2c].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  s_i2c_handles[i2c].Init.OwnAddress2 = 0;
  s_i2c_handles[i2c].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  s_i2c_handles[i2c].Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&s_i2c_handles[i2c]) != HAL_OK) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  if (__HAL_I2C_GET_FLAG(&s_i2c_handles[i2c], I2C_FLAG_BUSY) == SET) {
    return STATUS_CODE_UNREACHABLE;
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS) {
    return STATUS_CODE_INVALID_ARGS;
  } else if (settings->speed >= NUM_I2C_SPEEDS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_port[i2c].settings = *settings;

  // Enable clock for I2C
  switch (s_port[i2c].periph) {
    case RCC_APB1ENR1_I2C1EN:
      __HAL_RCC_I2C1_CLK_ENABLE();
      break;
    case RCC_APB1ENR1_I2C2EN:
      __HAL_RCC_I2C2_CLK_ENABLE();
      break;
    default:
      break;
  }

  // Enable clock for GPIOB
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // NOTE(mitch): This shouldn't be required for I2C, was needed to get around
  // Issue with SCL not being set high
  gpio_init_pin(&(settings->scl), GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&(settings->sda), GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);

  // Initialize pins to correct mode to operate I2C (open drain)
  // SCL should be open drain as well, but this was not working with hardware
  // Since we only use one master shouldn't be an issue
  gpio_init_pin(&(settings->scl), GPIO_ALTFN_PUSH_PULL, GPIO_STATE_HIGH);
  gpio_init_pin(&(settings->sda), GPIO_ALFTN_OPEN_DRAIN, GPIO_STATE_HIGH);

  s_i2c_handles[i2c].Instance = s_port[i2c].base;
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
  __HAL_I2C_DISABLE_IT(&s_i2c_handles[i2c], I2C_IT_ERRI | I2C_IT_TCI | I2C_IT_STOPI | I2C_IT_NACKI |
                                                I2C_IT_ADDRI | I2C_IT_RXI | I2C_IT_TXI);

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

// Do a transaction on the I2C bus
static StatusCode prv_txn(I2CPort i2c, I2CAddress addr, uint8_t *data, size_t len, bool read) {
  // Check that bus is not busy - If it is, assume that lockup has occurred
  StatusCode status = STATUS_CODE_OK;
  if (!s_port[i2c].multi_txn && __HAL_I2C_GET_FLAG(&s_i2c_handles[i2c], I2C_FLAG_BUSY) == SET) {
    status_ok_or_return(prv_recover_lockup(i2c));
  }
  queue_reset(&s_port[i2c].i2c_buf.queue);
  s_port[i2c].current_addr = (addr) << 1;

  // Set up txn for read or write
  if (read) {
    s_port[i2c].curr_mode = I2C_MODE_RECEIVE;
    s_port[i2c].num_rx_bytes = len;
  } else {
    s_port[i2c].curr_mode = I2C_MODE_TRANSMIT;
    for (size_t tx = 0; tx < len; tx++) {
      if (queue_send(&s_port[i2c].i2c_buf.queue, &data[tx], 0)) {
        return STATUS_CODE_RESOURCE_EXHAUSTED;
      }
    }
  }

  // Enable Interrupts before setting start to begin txn
  s_port[i2c].exit = STATUS_CODE_OK;
  HAL_I2C_Master_Transmit_IT(&s_i2c_handles[i2c], addr, data, len);
  HAL_I2C_Master_Receive_IT(&s_i2c_handles[i2c], addr, data, len);
  HAL_I2C_EnableListen_IT(&s_i2c_handles[i2c]);
  // Wait for signal that txn is finished from ISR, then disable IT and generate stop
  status = sem_wait(&s_port[i2c].i2c_buf.wait_txn, I2C_TIMEOUT_MS);
  HAL_I2C_DisableListen_IT(&s_i2c_handles[i2c]);
  if (!s_port[i2c].multi_txn ||
      status) {  // In an multi exchange or failed txn, need to keep line live to receive data
    HAL_I2C_Master_Abort_IT(&s_i2c_handles[i2c], addr);
  }
  status_ok_or_return(s_port[i2c].exit);
  if (read) {
    // Receive data from queue
    // If less than requested is received, an error in the transaction has occurred
    for (size_t rx = 0; rx < len; rx++) {
      if (queue_receive(&s_port[i2c].i2c_buf.queue, &data[rx], 0)) {
        return STATUS_CODE_INTERNAL_ERROR;
      }
    }
  }
  return STATUS_CODE_OK;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS || rx_len > I2C_MAX_NUM_DATA) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // Lock I2C resource
  StatusCode res = STATUS_CODE_OK;
  status_ok_or_return(mutex_lock(&s_port[i2c].mutex, I2C_TIMEOUT_MS));
  s_port[i2c].multi_txn = false;
  res = prv_txn(i2c, addr, rx_data, rx_len, true);
  mutex_unlock(&s_port[i2c].mutex);
  // Return status code ok unless an error has occurred
  return res;
}

// Address needs to be just the device address, read/write bit is taken care of in hardware
StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS || tx_len > I2C_MAX_NUM_DATA) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // Lock I2C resource
  StatusCode res = STATUS_CODE_OK;
  status_ok_or_return(mutex_lock(&s_port[i2c].mutex, I2C_TIMEOUT_MS));
  s_port[i2c].multi_txn = false;
  res = prv_txn(i2c, addr, tx_data, tx_len, false);
  mutex_unlock(&s_port[i2c].mutex);
  return res;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                        size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS || rx_len > I2C_MAX_NUM_DATA) {
    return STATUS_CODE_INVALID_ARGS;
  }

  StatusCode res = STATUS_CODE_OK;
  status_ok_or_return(mutex_lock(&s_port[i2c].mutex, I2C_TIMEOUT_MS));
  uint8_t reg_to_read = reg;
  s_port[i2c].multi_txn = true;
  // Write register value, then read result in second txn
  res = prv_txn(i2c, addr, &reg_to_read, sizeof(reg_to_read), false);
  if (res) {
    mutex_unlock(&s_port[i2c].mutex);
    return res;
  }
  res = prv_txn(i2c, addr, rx_data, rx_len, true);
  mutex_unlock(&s_port[i2c].mutex);
  return res;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS || tx_len > I2C_MAX_NUM_DATA - 1) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t write_data[I2C_MAX_NUM_DATA] = { 0 };
  write_data[0] = reg;
  for (size_t i = 1; i < tx_len + 1; i++) {
    write_data[i] = tx_data[i - 1];
  }

  status_ok_or_return(i2c_write(i2c, addr, write_data, tx_len + 1));
  return STATUS_CODE_OK;
}
