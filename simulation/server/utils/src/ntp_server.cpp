#include "ntp_server.h"

#include <netdb.h>

#include <cstring>
#include <iostream>

bool NTPServer::queryNTPServer(NTPPacket &response) {
  struct addrinfo hints = {}, *addrs;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  if (getaddrinfo(m_NTPServerAddress.c_str(), "123", &hints, &addrs) != 0) {
    freeaddrinfo(addrs);
    throw std::runtime_error("DNS resolution failed");
  }

  int ntpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (ntpSocket < 0) {
    std::cerr << "Error creating socket" << std::endl;
    return false;
  }

  NTPPacket request = {};
  request.flags = (NTP_VERSION << NTP_VERSION_OFFSET) | (NTPLeapIndicator::NTP_LI_NOSYNC << NTP_LEAP_INDICATOR_OFFSET) |
                  (NTPMode::NTP_CLIENT_MODE << NTP_MODE_OFFSET);

  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto nanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

  request.transmitTime.seconds = seconds.count() + NTP_UNIX_EPOCH_DIFF;
  request.transmitTime.fraction = static_cast<uint32_t>((nanoSeconds.count() % 1000000000ULL) * 4.294967296);

  if (sendto(ntpSocket, &request, sizeof(request), 0, addrs->ai_addr, addrs->ai_addrlen) < 0) {
    close(ntpSocket);
    freeaddrinfo(addrs);
    throw std::runtime_error("Send failed");
  }

  NTPPacket serverResponse = {};
  sockaddr_storage serverAddr;
  socklen_t serverAddrLen = sizeof(serverAddr);

  ssize_t receivedBytes = recvfrom(ntpSocket, &serverResponse, sizeof(serverResponse), 0, reinterpret_cast<sockaddr *>(&serverAddr), &serverAddrLen);

  freeaddrinfo(addrs);

  if (receivedBytes < 0) {
    close(ntpSocket);
    throw std::runtime_error("Receive timeout or error");
  }

  convertNTPTimestamp(serverResponse.referenceTime);
  convertNTPTimestamp(serverResponse.originTime);
  convertNTPTimestamp(serverResponse.receiveTime);
  convertNTPTimestamp(serverResponse.transmitTime);

  serverResponse.rootDelay = ntohl(serverResponse.rootDelay);
  serverResponse.rootDispersion = ntohl(serverResponse.rootDispersion);

  response = serverResponse;

  close(ntpSocket);
  return true;
}

NTPPacket NTPServer::processNTPRequest(const NTPPacket &request) {
  NTPPacket response = {};
  response.flags = (NTP_VERSION << NTP_VERSION_OFFSET) | (NTPLeapIndicator::NTP_LI_NONE << NTP_LEAP_INDICATOR_OFFSET) |
                   (NTPMode::NTP_SERVER_MODE << NTP_MODE_OFFSET);

  response.stratum = 2U;
  response.poll = 4U;      /* 2^4 = 16 seconds between succesive messages. */
  response.precision = -6; /* 2^-6 = 15.625 microseconds precision */
  response.rootDelay = htonl(0U);
  response.rootDispersion = htonl(0U);

  /* Local reference Id */
  response.referenceId[0] = 'L';
  response.referenceId[1] = 'O';
  response.referenceId[2] = 'C';
  response.referenceId[3] = 'L';

  auto now = std::chrono::system_clock::now();
  uint32_t currentSeconds = unixToNTPTime(std::chrono::system_clock::to_time_t(now));
  uint32_t currentFraction = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000);

  response.referenceTime.seconds = htonl(currentSeconds);
  response.referenceTime.fraction = htonl(currentFraction);
  response.originTime.seconds = ntohl(request.transmitTime.seconds);
  response.originTime.fraction = ntohl(request.transmitTime.fraction);
  response.receiveTime.seconds = htonl(currentSeconds);
  response.receiveTime.fraction = htonl(currentFraction);
  response.transmitTime.seconds = htonl(currentSeconds);
  response.transmitTime.fraction = htonl(currentFraction);

  NTPPacket externalResponse;
  if (queryNTPServer(externalResponse)) {
    response.referenceTime = externalResponse.referenceTime;
    response.originTime = externalResponse.originTime;
    response.receiveTime = externalResponse.receiveTime;
    response.transmitTime = externalResponse.transmitTime;
  }

  return response;
}

void NTPServer::NTPServerProcedure() {
  m_NTPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (m_NTPSocket < 0) {
    throw std::runtime_error("Error creating socket for NTP server: " + m_NTPServerAddress);
  }

  int enable = 1;
  if (setsockopt(m_NTPSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    throw std::runtime_error("Error setting socket option SO_REUSEADDR");
  }

  struct sockaddr_in serverAddr;
  memset((char *)&serverAddr, 0U, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(NTP_PORT);

  if (inet_pton(AF_INET, m_bindAddress.c_str(), &serverAddr.sin_addr) <= 0) {
    close(m_NTPSocket);
    throw std::runtime_error("Error converting IPv4 bind address to binary form");
  }

  if (bind(m_NTPSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    close(m_NTPSocket);
    throw std::runtime_error("Error binding NTP socket");
  }

  m_isListening = true;

  while (m_isListening) {
    NTPPacket request;
    struct sockaddr_in clientAddress;
    socklen_t clientLength = sizeof(clientAddress);

    if (recvfrom(m_NTPSocket, &request, sizeof(request), 0, (struct sockaddr *)&clientAddress, &clientLength) <= 0) {
      std::cerr << "Error receiving NTP request" << std::endl;
      continue;
    }

    NTPPacket response = processNTPRequest(request);
    dumpNTPPacketData(response);
    if (sendto(m_NTPSocket, &response, sizeof(response), 0, (struct sockaddr *)&clientAddress, clientLength) <= 0) {
      std::cerr << "Error sending NTP response" << std::endl;
    }
  }

  m_isListening = false;
  close(m_NTPSocket);
}

void *NTPServerWrapper(void *param) {
  NTPServer *server = static_cast<NTPServer *>(param);

  try {
    server->NTPServerProcedure();
  } catch (std::exception &e) {
    std::cerr << "NTP Server Thread Error: " << e.what() << std::endl;
  }

  return nullptr;
}

void NTPServer::startListening(const std::string &bindAddress, const std::string &NTPServerAddress) {
  if (m_isListening)
    return;

  m_NTPServerAddress = NTPServerAddress;
  m_bindAddress = bindAddress;

  if (pthread_create(&m_NTPServerThreadId, nullptr, NTPServerWrapper, this)) {
    throw std::runtime_error("Listen Error");
  }
}

NTPServer::NTPServer() {
  m_isListening = false;
  m_NTPSocket = -1;
}
