#pragma once

/************************************************************************************************
 * @file   imu_hw_defs.h
 *
 * @brief  Header file for imu hardware definitions
 *
 * @date   2025-03-06
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup imu
 * @brief    imu Firmware
 * @{
 */

/************************************************************************************************
 * IMU CAN definitions
 ************************************************************************************************/

/** @brief  IMU CAN RX Port */
#define IMU_CAN_RX \
  { .port = GPIO_PORT_B, .pin = 8 }

/** @brief  IMU CAN TX Port */
#define IMU_CAN_TX \
  { .port = GPIO_PORT_B, .pin = 9 }

/************************************************************************************************
 * IMU SPI definitions
 ************************************************************************************************/

#define IMU_SPI_PORT SPI_PORT_2

#define IMU_SPI_SDO \
  { .port = GPIO_PORT_B, .pin = 15 }

#define IMU_SPI_SDI \
  { .port = GPIO_PORT_B, .pin = 14 }

#define IMU_SPI_NSS \
  { .port = GPIO_PORT_B, .pin = 12 }

#define IMU_SPI_SCLK \
  { .port = GPIO_PORT_B, .pin = 13 }

#define IMU_SPI_INT1 \
  { .port = GPIO_PORT_B, .pin = 0 }

#define IMU_SPI_INT2 \
  { .port = GPIO_PORT_B, .pin = 1 }


#define IMU_SPI_BAUDRATE  SPI_BAUDRATE_2_5MHZ

#define IMU_SPI_MODE      SPI_MODE_3

#define IMU_SPI_PORT      SPI_PORT_2

/** @} */
