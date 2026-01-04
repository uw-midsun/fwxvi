#include "bmi323.h"
#include "imu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"  // For LOG_ERROR
#include "datagram.h"
#include <string.h>  // For memcpy
#include "uart.h"

void imu_task(void *pvParameters) {
  Bmi323Storage *storage = (Bmi323Storage *)pvParameters;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(100);

  while (1) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);

    StatusCode status = bmi323_update(storage);
    if (status != STATUS_CODE_OK) {
      LOG_ERROR("status error");
      continue;
    }

    // Create buffer and send over UART
    uint8_t buffer[26];
    buffer[0] = DATAGRAM_START_FRAME;
    memcpy(&buffer[1], &storage->gyro.x, sizeof(float));
    memcpy(&buffer[5], &storage->gyro.y, sizeof(float));
    memcpy(&buffer[9], &storage->gyro.z, sizeof(float));
    memcpy(&buffer[13], &storage->accel.x, sizeof(float));
    memcpy(&buffer[17], &storage->accel.y, sizeof(float));
    memcpy(&buffer[21], &storage->accel.z, sizeof(float));
    buffer[25] = DATAGRAM_END_FRAME;

    uart_tx(UART_PORT_1, buffer, sizeof(buffer));
  }
}

StatusCode imu_init(Bmi323Storage *storage, Bmi323Settings *settings) {
  storage->settings = settings;

  StatusCode status = bmi323_init(storage);
  if (status != STATUS_CODE_OK) {
    return status;
  }

  xTaskCreate(imu_task, "imu", 1024, storage, 1, NULL);

  return STATUS_CODE_OK;
}
