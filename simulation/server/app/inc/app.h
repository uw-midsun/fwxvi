#ifndef APP_H
#define APP_H

#include <string>

#include "gpio_manager.h"
#include "json_manager.h"

#ifndef USE_NETWORK_TIME_PROTOCOL
#define USE_NETWORK_TIME_PROTOCOL 0U
#endif

extern JSONManager globalJSON;
extern GpioManager serverGpioManager;

#endif
