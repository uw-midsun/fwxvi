/************************************************************************************************
 * @file   app_globals.cc
 *
 * @brief  Global server application object definitions
 *
 * @date   2026-04-27
 * @author Midnight Sun Team
 ************************************************************************************************/

/* Intra-component Headers */
#include "app.h"

JSONManager serverJSONManager;
GpioManager serverGpioManager;
AfeManager serverAfeManager;
AdcManager serverAdcManager;
I2CManager serverI2CManager;
SPIManager serverSPIManager;
CanListener serverCanListener;
CanScheduler serverCanScheduler;
