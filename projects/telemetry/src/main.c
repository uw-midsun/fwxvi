/************************************************************************************************
 * @file   main.c
 *
 * @brief  Main file for telemetry
 *
 * @date   2025-01-25
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "can.h"
#include "datagram.h"
#include "gpio.h"
#include "log.h"
#include "master_tasks.h"
#include "mcu.h"
#include "tasks.h"
#include "uart.h"
#include "ws22_motor_can.h"

/* Intra-component Headers */
#include "imu.h"
#include "sd_card_spi.h"
#include "telemetry.h"
#include "telemetry_getters.h"
#include "telemetry_hw_defs.h"
#include "xb_transmit.h"

TelemetryStorage telemetry_storage;

TelemetryConfig telemetry_config = {
  .message_transmit_frequency_hz = 1000U,
  .uart_port = TELEMETRY_XBEE_UART_PORT,
  .uart_settings = { .tx = GPIO_TELEMETRY_UART_TX, .rx = GPIO_TELEMETRY_UART_RX, .baudrate = TELEMETRY_XBEE_UART_BAUDRATE, .flow_control = TELEMETRY_XBEE_UART_FLOW_CONTROL },
  .sd_spi_port = SD_SPI_PORT_2,
  .sd_spi_settings = { .baudrate = SD_SPI_BAUDRATE_312_5KHZ,
                       .mode = SD_SPI_MODE_0,
                       .mosi = GPIO_TELEMETRY_SPI_MOSI,
                       .miso = GPIO_TELEMETRY_SPI_MISO,
                       .sclk = GPIO_TELEMETRY_SPI_SCK,
                       .cs = GPIO_TELEMETRY_SD_CS,
                      },
};

Bmi323Settings bmi323_settings = {
  .spi_port = SPI_PORT_2,
  .spi_settings = { .baudrate = SPI_BAUDRATE_5MHZ,
                    .mode = SPI_MODE_3,
                    .sdo = GPIO_TELEMETRY_SPI_MOSI,
                    .sdi = GPIO_TELEMETRY_SPI_MISO,
                    .sclk = GPIO_TELEMETRY_SPI_SCK,
                    .cs = GPIO_TELEMETRY_IMU_CS,
                  },
  .accel_range = IMU_ACCEL_RANGE_2G,
  .gyro_range = IMU_GYRO_RANGE_500_DEG,
};

Bmi323Storage bmi323_storage = {
  .settings = &bmi323_settings,
};

CanStorage can_storage = { 0 };

Ws22MotorCanStorage ws22_storage = { 0 };
Ws22MotorCanConfig ws22_config = {
  .ws22_status_info_enabled = true,
  .ws22_bus_measurement_enabled = true,
  .ws22_velocity_measurement_enabled = true,
  .ws22_phase_current_enabled = true,
  .ws22_motor_voltage_enabled = true,
  .ws22_motor_current_enabled = true,
  .ws22_motor_back_emf_enabled = true,
  .ws22_rail_15v_enabled = true,
  .ws22_temperature_enabled = true,
  .ws22_drive_cmd_enabled = true,
};

float roll = 0;
float pitch = 0;
float yaw = 0;
static volatile bool s_telemetry_ready;

/* SD card writes might lowk be slow, putting in another task for now */
TASK(telemetry_sd_logger, TASK_STACK_2048) {
  while (true) {
    StatusCode status = telemetry_log_sd();
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("SD snapshot failed: %u\n", (unsigned)status);
    }
    vTaskDelay(pdMS_TO_TICKS(1000U));
  }
}

static void __attribute__((unused)) s_prepare_filter(void) {
  for (float i = 0; i < 1000; i++) {
    imu_filter(0.05, 0.05, 0.9, 0, 0, 0);
    eulerAngles(q_est, &roll, &pitch, &yaw);
  }
}

void pre_loop_init() {
  GpioAddress sd_cs = GPIO_TELEMETRY_SD_CS;
  GpioAddress imu_cs = GPIO_TELEMETRY_IMU_CS;

  StatusCode status = gpio_init_pin(&sd_cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  if (status != STATUS_CODE_OK) goto fail;
  status = gpio_init_pin(&imu_cs, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  if (status != STATUS_CODE_OK) goto fail;

  status = ws22_motor_can_init(&ws22_storage, &ws22_config);
  if (status != STATUS_CODE_OK) goto fail;
  telemetry_storage.ws22_storage = &ws22_storage;
  status = telemetry_init(&telemetry_storage, &telemetry_config, &bmi323_storage, &can_storage);
  if (status != STATUS_CODE_OK) goto fail;
  // s_prepare_filter();
  // xb_transmit_init(&telemetry_storage, &telemetry_config);
  status = tasks_init_task(telemetry_sd_logger, TASK_PRIORITY(1), NULL);
  if (status != STATUS_CODE_OK) goto fail;
  s_telemetry_ready = true;
  return;
fail:
  LOG_DEBUG("Telemetry initialization failed: %u\n", (unsigned)status);
}

void run_1000hz_cycle() {
  if (s_telemetry_ready) run_can_rx_all();
}

void run_10hz_cycle() {
  if (s_telemetry_ready) run_can_tx_medium();
}

void run_1hz_cycle() {
  if (s_telemetry_ready) run_can_tx_slow();
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  init_master_tasks();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
