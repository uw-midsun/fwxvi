#pragma once

/************************************************************************************************
 * @file   front_controller_hw_defs.h
 *
 * @brief  Header file for front controller HW definition
 *
 * @date   2025-07-19
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Front_Controller
 * @brief    Front Controller Board Firmware
 * @{
 */
/************************************************************************************************
 * File-scope definitions
 ************************************************************************************************/

#define FRONT_CONTROLLER_GPIO_DEF(PORT, PIN) \
  { .port = GPIO_PORT_##PORT, .pin = PIN }

/************************************************************************************************
 * Front Controller CAN definitions
 ************************************************************************************************/

/** @brief  Front Controller CAN RX Port */
#define GPIO_FRONT_CONTROLLER_CAN_RX FRONT_CONTROLLER_GPIO_DEF(B, 8)

/** @brief  Front Controller CAN TX Port */
#define GPIO_FRONT_CONTROLLER_CAN_TX FRONT_CONTROLLER_GPIO_DEF(B, 9)

/************************************************************************************************
 * Load switch definitions
 ************************************************************************************************/

/** @brief  Front Controller left signal load switch enable */
#define GPIO_FRONT_CONTROLLER_LEFT_SIG_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(B, 12)

/** @brief  Front Controller right signal load switch enable */
#define GPIO_FRONT_CONTROLLER_RIGHT_SIG_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(B, 13)

/** @brief  Front Controller brake light load switch enable */
#define GPIO_FRONT_CONTROLLER_BRAKE_LIGHT_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(A, 10)

/** @brief  Front Controller battery-protection-system light load switch enable */
#define GPIO_FRONT_CONTROLLER_BPS_LIGHT_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(A, 9)

/** @brief  Front Controller driver fan load switch enable */
#define GPIO_FRONT_CONTROLLER_DRIVER_FAN_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(B, 14)

/** @brief  Front Controller reverse camera load switch enable */
#define GPIO_FRONT_CONTROLLER_REV_CAM_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(B, 15)

/** @brief  Front Controller telemetry load switch enable */
#define GPIO_FRONT_CONTROLLER_TELEM_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(A, 8)

/** @brief  Front Controller steering load switch enable */
#define GPIO_FRONT_CONTROLLER_STEERING_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(B, 11)

/** @brief  Front Controller horn load switch enable */
#define GPIO_FRONT_CONTROLLER_HORN_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(A, 11)

/** @brief  Front Controller spare 12V load switch enable */
#define GPIO_FRONT_CONTROLLER_SPARE_1_LS_ENABLE FRONT_CONTROLLER_GPIO_DEF(A, 12)

/************************************************************************************************
 * Current sense mux definitions
 ************************************************************************************************/

/** @brief  Front Controller current sense mux select 0 */
#define GPIO_FRONT_CONTROLLER_MUX_SEL_2 FRONT_CONTROLLER_GPIO_DEF(A, 6)

/** @brief  Front Controller current sense mux select 1 */
#define GPIO_FRONT_CONTROLLER_MUX_SEL_3 FRONT_CONTROLLER_GPIO_DEF(A, 7)

/** @brief  Front Controller current sense mux select 2 */
#define GPIO_FRONT_CONTROLLER_MUX_SEL_0 FRONT_CONTROLLER_GPIO_DEF(B, 1)

/** @brief  Front Controller current sense mux select 3 */
#define GPIO_FRONT_CONTROLLER_MUX_SEL_1 FRONT_CONTROLLER_GPIO_DEF(B, 2)

/** @brief  Front Controller current sense mux input */
#define GPIO_FRONT_CONTROLLER_MUX_OUTPUT FRONT_CONTROLLER_GPIO_DEF(B, 0)

/************************************************************************************************
 * Pedal definitions
 ************************************************************************************************/

/** @brief  Front Controller accelerator pedal input */
#define GPIO_FRONT_CONTROLLER_ACCEL_PEDAL FRONT_CONTROLLER_GPIO_DEF(A, 3)

/** @brief  Front Controller brake pedal input */
#define GPIO_FRONT_CONTROLLER_BRAKE_PEDAL FRONT_CONTROLLER_GPIO_DEF(A, 5)

/************************************************************************************************
 * Board LED definition
 ************************************************************************************************/

/** @brief  Front Controller accelerator pedal input */
#define GPIO_FRONT_CONTROLLER_BOARD_LED FRONT_CONTROLLER_GPIO_DEF(B, 10)

/** @} */
