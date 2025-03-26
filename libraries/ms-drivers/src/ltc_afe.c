#include "ltc_afe.h"

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_write_config(LtcAfeStorage *afe) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_trigger_cell_conv(LtcAfeStorage *afe) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_trigger_aux_conv(LtcAfeStorage *afe, uint8_t device_cell) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_read_cells(LtcAfeStorage *afe) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_read_aux(LtcAfeStorage *afe, uint8_t device_cell) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode ltc_afe_set_discharge_pwm_cycle(LtcAfeStorage *afe, uint8_t duty_cycle) {
  return STATUS_CODE_UNIMPLEMENTED;
}
