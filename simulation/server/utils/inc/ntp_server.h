#ifndef NTP_SERVER_H
#define NTP_SERVER_H

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <ctime>
#include <string>

#include "network_time_protocol.h"

#define NTP_SYNC_PERIOD_S 60

class NTPServer {
 private:
  static constexpr int NTP_PORT = 123;
  int m_NTPSocket;
  pthread_t m_NTPServerThreadId;
  std::atomic<bool> m_isListening;
  std::string m_bindAddress;
  std::string m_NTPServerAddress;

  NTPPacket processNTPRequest(const NTPPacket &request);
  bool queryNTPServer(NTPPacket &response);

 public:
  NTPServer();

  void NTPServerProcedure();
  void startListening(const std::string &bindAddress, const std::string &NTPServerAddress);
};

#endif
