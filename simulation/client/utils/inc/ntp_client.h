#pragma once

/************************************************************************************************
 * @file   ntp_client.h
 *
 * @brief  Header file defining the NTPClient class
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
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network_time_protocol.h"

/* Intra-component Headers */

/**
 * @defgroup Client_Utils
 * @brief    Client Utilities and Infrastructure
 * @{
 */

/**
 * @class   NTPClient
 * @brief   Class that represents the netowrk-time-protocol client that synchronizes with a server
 * @details This class is responsible for fetching stratum 2 time and synchronizing itself
 *          The client shall account for timing errors/differences every NTP_SYNC_PERIOD_S
 */
class NTPClient {
 private:
  static constexpr int NTP_PORT = 123;         /**< Universal port for network-time-protocol */
  static constexpr int NTP_SYNC_PERIOD_S = 60; /**< Client synchronization period */

  int m_NTPSocket;                     /**< The NTP servers socket FD */
  std::string m_serverAddress;         /**< The NTP servers address for fetching stratum 2 data (ie: 127.0.0.1) */
  NTPPacket m_serverResponse;          /**< NTP Packet to store the servers timing data */
  pthread_t m_NTPClientThreadId;       /**< Thread Id for synchronizing the client */
  std::atomic<bool> m_isSynchronizing; /**< Boolean flag to indicate the synchronization status of the client */

 public:
  /**
   * @brief   Constructs a NTPClient object
   * @details Initializes the NTP client. The constructor sets up internal variables
   */
  NTPClient();

  /**
   * @brief   Thread procedure for handling NTP synchronization
   * @details This thread shall run at the NTP_SYNC_PERIOD_S frequency
   */
  void NTPClientProcedure();

  /**
   * @brief   Starts NTP synchronization of the client
   * @details This function will spawn the NTPClientProcedure thread
   * @param   serverAddress The address of the NTP server to bind to
   */
  void startSynchronization(const std::string &serverAddress);
};

/** @} */
