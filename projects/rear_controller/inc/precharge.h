#pragma once

#include "gpio_interrupts.h"
#include "notify.h"
#include "tasks.h"


#define PRECHARGE_EVENT 0 

StatusCode precharge_init (Event event, const Task *task);