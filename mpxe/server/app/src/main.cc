/************************************************************************************************
 * @file   main.cc
 *
 * @brief  Main source file for app
 *
 * @date   2025-08-18
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

/* Handler function defintions (imported from app_terminal.cc) */
std::string handleGpioCommands(const std::string &action, std::vector<std::string> &tokens);

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
      std::vector<std::string> tokens; 
      std::string command; 
        
      tokens = {"GPIO", "SET_ALL_STATES", "HIGH" };
      command = "set_all_states";
      message = handleGpioCommands(command, tokens);
      
      Server.broadcastMessage(message);
        
      tokens = {"GPIO", "GET_PIN_MODE", "A12" };
      command = "get_pin_mode";
      message = handleGpioCommands(command, tokens);
      
      Server.broadcastMessage(message);
      
      std::cout << "Battery voltage updated" << std::endl;   
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
  });
  task_threads.emplace_back([&Server]() {
    while(true){
      std::string message;
      std::vector<std::string> tokens; 
      std::string command; 
        
      tokens = {"GPIO", "GET_ALL_STATES" };
      command = "get_all_states";
      message = handleGpioCommands(command, tokens);
      
      Server.broadcastMessage(message);
        
      tokens = {"GPIO", "SET_PIN_STATE", "A12", "HIGH" };
      command = "set_pin_state";
      message = handleGpioCommands(command, tokens);
      
      Server.broadcastMessage(message);
      
      std::cout << "Thermistor voltage updated" << std::endl;   
      std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
  });
  task_threads.emplace_back([&Server]() {
    while(true){
      std::string message;
      std::vector<std::string> tokens; 
      std::string command; 
      
      /* CAN: SET afe1_status_temp 29 */
      serverCanScheduler.update_afe1_status_temp(29);
      /* CAN: SET afe2_status_temp 28 */
      serverCanScheduler.update_afe2_status_temp(28);
      /* CAN: SET afe3_status_temp 30 */
      serverCanScheduler.update_afe3_status_temp(30);
      std::cout << "AFE temps simulated" << std::endl;   
      std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
  });
  
  for (std::thread &t : task_threads){
      t.detach();
  }
  
  applicationTerminal.run();
  return 0;
}

std::string handleGpioCommands(const std::string &action, std::vector<std::string> &tokens) {
  std::string message;
  if (action == "get_pin_state" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_PIN_STATE, tokens[2], "");
  } else if (action == "get_all_states" && tokens.size() >= 2) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_ALL_STATES, tokens[0], "");
  } else if (action == "get_pin_mode" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_PIN_MODE, tokens[2], "");
  } else if (action == "get_all_modes" && tokens.size() >= 2) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_ALL_MODES, tokens[0], "");
  } else if (action == "get_pin_alt_function" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_PIN_ALT_FUNCTION, tokens[2], "");
  } else if (action == "get_all_alt_functions" && tokens.size() >= 2) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_GET_ALL_ALT_FUNCTIONS, tokens[0], "");
  } else if (action == "set_pin_state" && tokens.size() >= 4) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_SET_PIN_STATE, tokens[2], tokens[3]);
  } else if (action == "set_all_states" && tokens.size() >= 3) {
    message = serverGpioManager.createGpioCommand(CommandCode::GPIO_SET_ALL_STATES, tokens[0], tokens[2]);
  } else {
    std::cerr << "Unsupported action: " << action << std::endl;
  }
  return message;
}
