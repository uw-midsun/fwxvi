#include "precharge.h"

#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "master_task.h"
#include "motor_controller_getters.h"
#include "motor_controller_setters.h"
#include "status.h"

StatusCode precharge_init(const PrechargeSettings *settings, Event event, const Task *task) {
  gpio_init_pin(&settings->motor_sw, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&settings->precharge_monitor, GPIO_INPUT_FLOATING, GPIO_STATE_HIGH);

  GpioState state;
  gpio_get_state(&settings->precharge_monitor, &state);
  if (status != STATUS_CODE_OK) {
    LOG_WARN("Error getting monitor state\n");
    return status;
  }
  if (state == GPIO_STATE_HIGH) {
    set_mc_status_precharge_status(true);
    gpio_set_state(&settings->motor_sw, GPIO_STATE_HIGH);
    return STATUS_CODE_OK;
  } else if (state == GPIO_STATE_LOW) {
  
   InterruptSettings monitor_it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .edge = INTERRUPT_EDGE_RISING,
  };
  set_mc_status_precharge_status(false);
  return gpio_it_register_interrupt(&settings->precharge_monitor, &monitor_it_settings, event,
    task);
  }
  else{
    LOG_WARN("Unexpected error\n");
    return STATUS_CODE_INVALID_ARGS;  }


 
}