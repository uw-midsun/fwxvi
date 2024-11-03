/************************************************************************************************
 * gpio.c
 *
 * GPIO Library Source Code
 *
 * Created: 2024-11-02
 * Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "gpio.h"

#include "gpio_mcu.h"
#include "status.h"

StatusCode gpio_init(void) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioMode pin_mode,
                         GpioState init_state) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode gpio_set_state(const GpioAddress *address, GpioState state) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode gpio_toggle_state(const GpioAddress *address) {
  return STATUS_CODE_UNIMPLEMENTED;
}

GpioState gpio_get_state(const GpioAddress *address) {
  return GPIO_STATE_LOW;
}
