/************************************************************************************************
 * @file   main.cc
 *
 * @brief  Main source file for app
 *
 * @date   2025-08-01
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
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
CanListener serverCanListener;
CanScheduler serverCanScheduler;
GpioManager serverGpioManager;

int main(int argc, char **argv) {
  std::cout << "Running Server" << std::endl;
  Server Server;
  Terminal applicationTerminal(&Server);

  Server.listenClients(8080, applicationMessageCallback, applicationConnectCallback);

#if USE_NETWORK_TIME_PROTOCOL == 1U
  NTPServer ntp_server;
  ntp_server.startListening("127.0.0.1", "time.google.com");
#endif

  serverCanListener.listenCanBus();
  serverCanScheduler.startCanScheduler();

  /* ------ can_defaults ------ */
  serverCanScheduler.update_battery_vt_voltage(15000);
  serverCanScheduler.update_battery_vt_current(45000);
  serverCanScheduler.update_battery_vt_batt_perc(79);

  
  /* ------ tasks ------ */ 
  std::vector<std::thread> task_threads;
  task_threads.emplace_back([&Server]() {
    while(true){
      std::string message;
      std::string token_1;
      std::string token_2;
       
      token_1 =  "HIGH";
      token_2 = "";
      message = serverGpioManager.createGpioCommand(CommandCode::GPIO_SET_ALL_STATES, token_2, token_1);
      
      Server.broadcastMessage(message);
       
      token_1 =  "A12";
      token_2 = "";
      message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_PIN_MODE, token_1, token_2);
      
      Server.broadcastMessage(message);
      
      std::cout << "Battery voltage updated" << std::endl;   
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
  });
  task_threads.emplace_back([&Server]() {
    while(true){
      std::string message;
      std::string token_1;
      std::string token_2;
       
      token_1 =  " ";
      token_2 = "";
      message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_ALL_STATES, token_1, token_2);
      
      Server.broadcastMessage(message);
       
      token_1 =  "A12";
      token_2 = "HIGH";
      message = serverGpioManager.createGpioCommand(CommandCode::GPIO_SET_PIN_STATE, token_2, token_1);
      
      Server.broadcastMessage(message);
      
      std::cout << "Thermistor voltage updated" << std::endl;   
      std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
  });
  
  for (std::thread &t : task_threads){
      t.detach();
  }
  
  applicationTerminal.run();
  return 0;
}