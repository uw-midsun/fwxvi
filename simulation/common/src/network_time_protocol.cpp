#include "network_time_protocol.h"

#include <arpa/inet.h>

#include <cstring>
#include <iostream>

time_t ntpToUnixTime(uint32_t ntpTime) {
  return ntpTime - NTP_UNIX_EPOCH_DIFF;
}

uint32_t unixToNTPTime(time_t unixTime) {
  return unixTime + NTP_UNIX_EPOCH_DIFF;
}

void convertNTPTimestamp(NTPTime &timestamp) {
  timestamp.seconds = ntohl(timestamp.seconds);
  timestamp.fraction = ntohl(timestamp.fraction);
}

void dumpNTPPacketData(const NTPPacket packet) {
  uint8_t leapIndicator = (packet.flags >> NTP_LEAP_INDICATOR_OFFSET) & 0x03U;
  uint8_t version = (packet.flags >> NTP_VERSION_OFFSET) & 0x07U;
  uint8_t mode = packet.flags & 0x07U;

  const char *leapStrings[] = {"No warning", "Last minute has 61 seconds", "Last minute has 59 seconds", "Alarm: clock not synchronized"};

  const char *modeStrings[] = {"Reserved", "Symmetric Active", "Symmetric Passive",   "Client",
                               "Server",   "Broadcast",        "NTP Control Message", "Private Use"};

  std::cout << "NTP Packet Dump:" << std::endl;
  std::cout << "----------------" << std::endl;

  std::cout << "Leap Indicator: " << leapStrings[leapIndicator] << " (" << static_cast<int>(leapIndicator) << ")" << std::endl;
  std::cout << "NTP Version:    " << static_cast<int>(version) << std::endl;
  std::cout << "Mode:           " << modeStrings[mode] << " (" << static_cast<int>(mode) << ")" << std::endl;

  std::cout << "Stratum:        " << static_cast<int>(packet.stratum);
  switch (packet.stratum) {
    case 0:
      std::cout << " (Unspecified)";
      break;
    case 1:
      std::cout << " (Primary Reference)";
      break;
    case 2 ... 15:
      std::cout << " (Secondary Reference)";
      break;
    case 16 ... 255:
      std::cout << " (Reserved)";
      break;
  }
  std::cout << std::endl;

  auto formatTimestamp = [](NTPTime ts) {
    time_t unixTime = ntpToUnixTime(ts.seconds);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::gmtime(&unixTime));
    return std::string(buffer);
  };

  std::cout << "Reference Time: " << formatTimestamp(packet.referenceTime) << " (Seconds: " << packet.referenceTime.seconds
            << ", Fraction: " << packet.referenceTime.fraction << ")" << std::endl;

  std::cout << "Origin Time:    " << formatTimestamp(packet.originTime) << " (Seconds: " << packet.originTime.seconds
            << ", Fraction: " << packet.originTime.fraction << ")" << std::endl;

  std::cout << "Receive Time:   " << formatTimestamp(packet.receiveTime) << " (Seconds: " << packet.receiveTime.seconds
            << ", Fraction: " << packet.receiveTime.fraction << ")" << std::endl;

  std::cout << "Transmit Time:  " << formatTimestamp(packet.transmitTime) << " (Seconds: " << packet.transmitTime.seconds
            << ", Fraction: " << packet.transmitTime.fraction << ")" << std::endl;

  std::cout << "Poll Interval:  " << static_cast<int>(packet.poll) << " (2^" << static_cast<int>(packet.poll) << " seconds)" << std::endl;

  std::cout << "Precision:      " << static_cast<int>(packet.precision) << " (2^" << static_cast<int>(packet.precision) << " seconds)" << std::endl;

  std::cout << "Reference ID:   ";
  char refIdStr[5];
  memcpy(refIdStr, &packet.referenceId, 4);
  refIdStr[4] = '\0';
  std::cout << refIdStr << " (Raw: 0x" << std::hex << packet.referenceId << std::dec << ")" << std::endl;

  std::cout << "Root Delay:     " << (packet.rootDelay / 65536.0) << " seconds" << std::endl;
  std::cout << "Root Dispersion:" << (packet.rootDispersion / 65536.0) << " seconds" << std::endl;
}
