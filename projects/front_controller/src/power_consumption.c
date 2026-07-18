#include "power_consumption.h"

static FrontControllerStorage* s_front_controller_storage;

StatusCode power_consumption_init(FrontControllerStorage* front_controller_storage) {
  if(front_controller_storage == NULL) {
	return STATUS_CODE_OK;
  }

  front_controller_storage->power_consumption_storage->last_start = xTaskGetTickCount();
  *s_front_controller_storage = front_controller_storage;
  front_controller_storage->power_consumption_storage->power_consumed_wh = front_controller_storage->pedal_persist_data->power_consumption_wh;

  return STATUS_CODE_OK;
}

StatusCode power_consumption_run() {
  if(s_front_controller_storage == NULL) {
	return STATUS_CODE_OK;
  }

  TickType_t now = xTaskGetTickCount();

  TickType_t elapsed = now - s_front_controller_storage->power_consumption_storage->last_start;

  int32_t elapsed_ms = (int32_t)((int16_t)((uint16_t)elapsed / portTICK_PERIOD_MS) << 16U);
  int32_t elapsed_hr = elapsed_ms / ((3600 * 1000) << 16U);

  int32_t power = ((int16_t)get_battery_stats_B_pack_current_a() * (int16_t)get_battery_stats_A_pack_voltage_v) << 16U;

  int32_t consumption_wh = power * elapsed_hr;

  s_front_controller_storage->power_consumption_storage->power_consumed_wh += consumption_wh;
  s_front_controller_storage->power_consumption_storage->last_start = now;

  return STATUS_CODE_OK;
}
