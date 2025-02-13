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

#define CMD 0x7E
#define WRITE 0x7F
#define READ 0x80
#define FEATURE_IO0 0x10
#define FEATURE_IO1 0x11
#define FEATURE_IO2 0x12
#define FEATURE_IO3 0x13
#define FEATURE_IO_STATUS 0x14
#define FEATURE_CTRL 0x40
#define INT_STATUS_INT1 0x0D
#define INT_STATUS_INT2 0x0E



#define DUMMY_BYTE 0x00
#define BMI_SET_LOW_BYTE 0x00FF
#define BMI_SET_HIGH_BYTE 0xFF00
#define BMI3_E_NULL_PTR INT8_C(-1)



/*
this might be 13 or 14 idk
#define BMI3_ACC_DP_OFF_XYZ_13_BIT_MASK              UINT16_C(0x1FFF)
#define BMI3_ACC_DP_OFF_XYZ_14_BIT_MASK              UINT16_C(0x3FFF)
*/
#define BMI3_ACC_DP_DOFFSET_X_MASK UINT16_C(0xFFFF)
#define BMI3_ACC_DP_DOFFSET_Y_MASK UINT16_C(0xFFFF)
#define BMI3_ACC_DP_DOFFSET_Z_MASK UINT16_C(0xFFFF)

#define BMI3_ACC_DP_DGAIN_X_MASK UINT16_C(0x00FF)
#define BMI3_ACC_DP_DGAIN_Y_MASK UINT16_C(0x00FF)
#define BMI3_ACC_DP_DGAIN_Z_MASK UINT16_C(0x00FF)

#define BMI3_GYR_DP_OFF_X_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_X_MASK UINT16_C(0x007F)
#define BMI3_GYR_DP_OFF_Y_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_Y_MASK UINT16_C(0x007F)
#define BMI3_GYR_DP_OFF_Z_MASK UINT16_C(0x03FF)
#define BMI3_GYR_DP_DGAIN_Z_MASK UINT16_C(0x007F)

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