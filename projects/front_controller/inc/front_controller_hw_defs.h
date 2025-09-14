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
 * Front Controller CAN definitions
 ************************************************************************************************/

/** @brief  Front Controller CAN RX Port */
#define FRONT_CONTROLLER_CAN_RX \
  { .port = GPIO_PORT_B, .pin = 8 }

/** @brief  Front Controller CAN TX Port */
#define FRONT_CONTROLLER_CAN_TX \
  { .port = GPIO_PORT_B, .pin = 9 }

/************************************************************************************************
 * Load switch definitions
 ************************************************************************************************/

/** @brief  Front Controller headlight load switch enable */
#define FRONT_CONTROLLER_HEADLIGHT_LS_ENABLE \
  { .port = GPIO_PORT_A, .pin = 1 }

/** @brief  Front Controller left signal load switch enable */
#define FRONT_CONTROLLER_LEFT_SIG_LS_ENABLE \
  { .port = GPIO_PORT_A, .pin = 8 }

/** @brief  Front Controller right signal load switch enable */
#define FRONT_CONTROLLER_RIGHT_SIG_LS_ENABLE \
  { .port = GPIO_PORT_A, .pin = 9 }

/** @brief  Front Controller brake light load switch enable */
#define FRONT_CONTROLLER_BRAKE_LIGHT_LS_ENABLE \
  { .port = GPIO_PORT_A, .pin = 10 }

/** @brief  Front Controller battery-protection-system light load switch enable */
#define FRONT_CONTROLLER_BPS_LIGHT_LS_ENABLE \
  { .port = GPIO_PORT_A, .pin = 11 }

/** @brief  Front Controller driver fan load switch enable */
#define FRONT_CONTROLLER_DRIVER_FAN_LS_ENABLE \
  { .port = GPIO_PORT_A, .pin = 12 }

/** @brief  Front Controller reverse camera load switch enable */
#define FRONT_CONTROLLER_REV_CAM_LS_ENABLE \
  { .port = GPIO_PORT_A, .pin = 15 }

/** @brief  Front Controller telemetry load switch enable */
#define FRONT_CONTROLLER_TELEM_LS_ENABLE \
  { .port = GPIO_PORT_B, .pin = 2 }

/** @brief  Front Controller steering load switch enable */
#define FRONT_CONTROLLER_STEERING_LS_ENABLE \
  { .port = GPIO_PORT_B, .pin = 3 }

/** @brief  Front Controller horn load switch enable */
#define FRONT_CONTROLLER_HORN_LS_ENABLE \
  { .port = GPIO_PORT_B, .pin = 4 }

/** @brief  Front Controller spare 12V load switch enable */
#define FRONT_CONTROLLER_SPARE_1_LS_ENABLE \
  { .port = GPIO_PORT_B, .pin = 10 }

/** @brief  Front Controller spare 12V load switch enable */
#define FRONT_CONTROLLER_SPARE_2_LS_ENABLE \
  { .port = GPIO_PORT_B, .pin = 11 }

/************************************************************************************************
 * Current sense mux definitions
 ************************************************************************************************/

/** @brief  Front Controller current sense mux select 0 */
#define FRONT_CONTROLLER_MUX_SEL_0 \
  { .port = GPIO_PORT_B, .pin = 12 }

/** @brief  Front Controller current sense mux select 1 */
#define FRONT_CONTROLLER_MUX_SEL_1 \
  { .port = GPIO_PORT_B, .pin = 13 }

/** @brief  Front Controller current sense mux select 2 */
#define FRONT_CONTROLLER_MUX_SEL_2 \
  { .port = GPIO_PORT_B, .pin = 14 }

/** @brief  Front Controller current sense mux select 3 */
#define FRONT_CONTROLLER_MUX_SEL_3 \
  { .port = GPIO_PORT_B, .pin = 15 }

/** @brief  Front Controller current sense mux input */
#define FRONT_CONTROLLER_MUX_OUTPUT \
  { .port = GPIO_PORT_A, .pin = 2 }

/************************************************************************************************
 * Pedal definitions
 ************************************************************************************************/

/** @brief  Front Controller accelerator pedal input */
#define FRONT_CONTROLLER_ACCEL_PEDAL \
  { .port = GPIO_PORT_A, .pin = 3 }

  //Brake pedal port initialize 
#define FRONT_CONTROLLER_BRAKE_PEDAL \
  { .port = GPIO_PORT_A, .pin = 2}

/** @} */
