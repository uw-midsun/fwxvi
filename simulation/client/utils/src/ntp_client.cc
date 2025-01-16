/************************************************************************************************
 * @file   ntp_client.cc
 *
 * @brief  Source file defining the NTPClient class
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstring>
#include <iostream>

/* Inter-component Headers */
#include "thread_helpers.h"

/* Intra-component Headers */
#include "ntp_client.h"

void NTPClient::NTPClientProcedure() {
  int ntpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (ntpSocket < 0) {
    close(ntpSocket);
    throw std::runtime_error("Error creating socket");
  }

  struct sockaddr_in serverAddr;
  memset((char *)&serverAddr, 0U, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(NTP_PORT);

  if (inet_pton(AF_INET, m_serverAddress.c_str(), &serverAddr.sin_addr) <= 0) {
    close(ntpSocket);
    throw std::runtime_error("Invalid server address");
  }

  m_isSynchronizing = true;
  std::cerr << "Connected NTP Server" << std::endl;

  while (m_isSynchronizing) {
    NTPPacket request = {};
    request.flags = (NTP_VERSION << NTP_VERSION_OFFSET) | (NTPLeapIndicator::NTP_LI_NOSYNC << NTP_LEAP_INDICATOR_OFFSET) | (NTPMode::NTP_CLIENT_MODE << NTP_MODE_OFFSET);

    /* Convert UNIX time in seconds to NTP time */
    request.transmitTime.seconds = htonl(unixToNTPTime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));

    /* Obtain the fractional time */
    request.transmitTime.fraction = htonl(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 1000000));

    if (sendto(ntpSocket, &request, sizeof(request), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <= 0) {
      close(ntpSocket);
      throw std::runtime_error("Error sending NTP request");
    }

    NTPPacket serverResponse = {};
    if (recvfrom(ntpSocket, &serverResponse, sizeof(serverResponse), 0, nullptr, nullptr) <= 0) {
      close(ntpSocket);
      throw std::runtime_error("Error receiving NTP response");
    }

    convertNTPTimestamp(serverResponse.referenceTime);
    convertNTPTimestamp(serverResponse.originTime);
    convertNTPTimestamp(serverResponse.receiveTime);
    convertNTPTimestamp(serverResponse.transmitTime);

    serverResponse.rootDelay = ntohl(serverResponse.rootDelay);
    serverResponse.rootDispersion = ntohl(serverResponse.rootDispersion);

    m_serverResponse = serverResponse;

    dumpNTPPacketData(m_serverResponse);
    thread_sleep_s(1);
  }

  m_isSynchronizing = false;
  close(ntpSocket);
}

void *NTPClientWrapper(void *param) {
  NTPClient *client = static_cast<NTPClient *>(param);

  try {
    client->NTPClientProcedure();
  } catch (std::exception &e) {
    std::cerr << "NTP Client Thread Error: " << e.what() << std::endl;
  }

  return nullptr;
}

void NTPClient::startSynchronization(const std::string &serverAddress) {
  m_serverAddress = serverAddress;

  if (pthread_create(&m_NTPClientThreadId, nullptr, NTPClientWrapper, this)) {
    throw std::runtime_error("Listen Error");
  }
}

NTPClient::NTPClient() {
  m_isSynchronizing = false;
  m_NTPSocket = -1;
}
