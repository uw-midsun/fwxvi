#pragma once

/************************************************************************************************
 * @file   imu_hw_defs.h
 *
 * @brief  Header file for IMU hardware definitions
 *
 * @date   2025-01-12
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

#define IMU_SPI_MOSI \
{ .port = GPIO_PORT_B, .pin = 15 }

#define IMU_SPI_MISO \
{ .port = GPIO_PORT_B, .pin = 14 }

#define IMU_SPI_NSS \
{ .port = GPIO_PORT_B, .pin = 12 }

#define IMU_SPI_SCK \
{ .port = GPIO_PORT_B, .pin = 13 }

#define IMU_SPI_INT1 \
{ .port = GPIO_PORT_B, .pin = 0 }

#define IMU_SPI_INT2 \
{ .port = GPIO_PORT_B, .pin = 1 }

#define SPI_BAUDRATE 2500000