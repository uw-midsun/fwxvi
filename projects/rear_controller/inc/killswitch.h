#pragma once

#include "gpio_interrupts.h"
#include "notify.h"
#include "tasks.h"

#define KILLSWITCH_EVENT 1 

StatusCode killswitch_init (Event event, const Task *task);