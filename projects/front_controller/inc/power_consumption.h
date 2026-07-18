#pragma once

#include "status.h"
#include "front_controller.h"
#include "tasks.h"

typedef struct {
  int32_t power_consumed_wh;
  TickType_t last_start;
} PowerConsumptionStorage;

StatusCode power_consumption_init(FrontControllerStorage* power_consumption_storage);

StatusCode power_consumption_run();
