#pragma once

/************************************************************************************************
 * @file   app.h
 *
 * @brief  Header file defining the Application for the server
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <string>

/* Inter-component Headers */
#include "json_manager.h"

/* Intra-component Headers */
#include "adbms_afe_manager.h"
#include "adc_manager.h"
#include "can_listener.h"
#include "can_scheduler.h"
#include "gpio_manager.h"
#include "spi_manager.h"

/**
 * @defgroup ServerAppMain
 * @brief    Server Application Main Interface
 * @{
 */

#ifndef USE_NETWORK_TIME_PROTOCOL
/** @brief  Define to use network-time-protocol synchronization. 0: Disabled 1: Enabled */
#define USE_NETWORK_TIME_PROTOCOL 0U
#endif

extern JSONManager serverJSONManager;   /**< Global JSON Manager */
extern GpioManager serverGpioManager;   /**< Global GPIO Manager */
extern AfeManager serverAfeManager;     /**< Global AFE Manager */
extern AdcManager serverAdcManager;     /**< Global ADC Manager */
extern SPIManager serverSPIManager;     /**< Global SPI Manager */
extern CanListener serverCanListener;   /**< Global CAN Listener */
extern CanScheduler serverCanScheduler; /**< Global CAN Scheduler */
/** @} */
