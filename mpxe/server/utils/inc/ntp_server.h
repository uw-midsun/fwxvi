#pragma once

/************************************************************************************************
 * @file   ntp_server.h
 *
 * @brief  Header file defining the NTPServer class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <atomic>
#include <chrono>
#include <ctime>
#include <string>

/* Inter-component Headers */
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network_time_protocol.h"

/* Intra-component Headers */

/**
 * @defgroup Server_Utils
 * @brief    Server Utilities and Infrastructure
 * @{
 */

/**
 * @class   NTPServer
 * @brief   Class that represents the netowrk-time-protocol server that synchronizes clients
 * @details This class is responsible for fetching stratum 0/1 time and forwarding it to clients
 *          The server shall account for timing errors/differences
 *          The client shall be responsibile for adjusting their timing
 */
class NTPServer {
 private:
  static constexpr int NTP_PORT = 123;                   /**< Universal port for network-time-protocol */
  static constexpr unsigned int NTP_SYNC_PERIOD_S = 60U; /**< Client synchronization period */

  int m_NTPSocket;                 /**< The NTP servers socket FD */
  std::string m_bindAddress;       /**< The NTP servers bind address */
  std::string m_NTPServerAddress;  /**< The NTP servers address for fetching stratum 0/1 data (ie: time.google.com) */
  pthread_t m_NTPServerThreadId;   /**< Thread Id for synchronizing clients */
  std::atomic<bool> m_isListening; /**< Boolean flag to indicate the NTP servers status */

  /**
   * @brief   Processes an incoming NTP request and returns the response packet
   * @details This shall create a stratum 2 time image
   *          If the NTP server query is succesful, the times shall be updated
   * @param   request The incoming NTP request packet
   * @return  The NTP response packet
   */
  NTPPacket processNTPRequest(const NTPPacket &request);

  /**
   * @brief   Queries the remote NTP server and fills the response packet
   * @details This shall fetch stratum 0/1 timing data
   *          The response from the server will contain the most accurate time information
   * @param   response The NTP response packet that will be filled with data from the server
   * @return  True if the query was successful, otherwise false
   */
  bool queryNTPServer(NTPPacket &response);

 public:
  /**
   * @brief   Constructs a NTPServer object
   * @details Initializes the NTP server. The constructor sets up internal variables
   */
  NTPServer();

  /**
   * @brief   Thread procedure for handling NTP synchronization
   * @details This thread shall run at the NTP_SYNC_PERIOD_S frequency
   */
  void NTPServerProcedure();

  /**
   * @brief   Starts listening to NTP clients to begin synchronization
   * @details This function will spawn the NTPServerProcedure thread
   * @param   bindAddress The local address to bind the NTP server
   * @param   NTPServerAddress The external NTP server address to query for time data
   */
  void startListening(const std::string &bindAddress, const std::string &NTPServerAddress);
};

/** @} */
