/************************************************************************************************
 * @file   main.cc
 *
 * @brief  Main source file
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>
#include <string>

/* Inter-component Headers */
#include "client_connection.h"
#include "json_manager.h"
#include "ntp_server.h"
#include "server.h"

/* Intra-component Headers */
#include "app.h"
#include "app_callback.h"
#include "app_terminal.h"
#include "can_listener.h"
#include "can_scheduler.h"
#include "gpio_manager.h"

JSONManager serverJSONManager;
GpioManager serverGpioManager;
CanListener serverCanListener;
CanScheduler serverCanScheduler;

int main(int argc, char **argv) {
  std::cout << "Running Server" << std::endl;
  Server Server;
  Terminal applicationTerminal(&Server);

  Server.listenClients(1024, applicationMessageCallback, applicationConnectCallback);

  serverCanListener.listenCanBus();

  serverCanScheduler.startCanScheduler();

  serverCanScheduler.update_battery_vt_voltage(15000);
  serverCanScheduler.update_battery_vt_current(45000);
  serverCanScheduler.update_battery_vt_batt_perc(79);
  serverCanScheduler.update_battery_status_aux_batt_v(12000);

  serverCanScheduler.update_afe1_status_v1(42500);
  serverCanScheduler.update_afe1_status_v2(40512);
  serverCanScheduler.update_afe1_status_v3(41500);

  serverCanScheduler.update_afe2_status_v1(42356);
  serverCanScheduler.update_afe2_status_v2(39786);
  serverCanScheduler.update_afe2_status_v3(40020);

  serverCanScheduler.update_afe3_status_v1(41340);
  serverCanScheduler.update_afe3_status_v2(40520);
  serverCanScheduler.update_afe3_status_v3(39612);

  serverCanScheduler.update_afe1_status_temp(27);
  serverCanScheduler.update_afe2_status_temp(26);
  serverCanScheduler.update_afe3_status_temp(28);

#if USE_NETWORK_TIME_PROTOCOL == 1U
  ntp_server.startListening("127.0.0.1", "time.google.com");
  NTPServer ntp_server;
#endif

  applicationTerminal.run();

  return 0;
}
