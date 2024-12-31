#ifndef NETWORK_TIME_PROTOCOL_H
#define NETWORK_TIME_PROTOCOL_H

#include <cstdint>
#include <ctime>

#define NTP_VERSION 4U

#define NTP_LEAP_INDICATOR_OFFSET 6U
#define NTP_VERSION_OFFSET 3U
#define NTP_MODE_OFFSET 0U

#define NTP_POLL_TO_SECONDS(poll) (1U << poll)

static constexpr uint32_t NTP_UNIX_EPOCH_DIFF = 2208988800UL;

struct NTPTime {
  uint32_t seconds;
  uint32_t fraction;
};

struct NTPPacket {
  uint8_t flags;
  uint8_t stratum;
  uint8_t poll;
  uint8_t precision;
  uint32_t rootDelay;
  uint32_t rootDispersion;
  uint8_t referenceId[4];
  NTPTime referenceTime;
  NTPTime originTime;
  NTPTime receiveTime;
  NTPTime transmitTime;
};

enum NTPMode {
  NTP_RESERVED_MODE,
  NTP_ACTIVE_MODE,
  NTP_PASSIVE_MODE,
  NTP_CLIENT_MODE,
  NTP_SERVER_MODE,
  NTP_BROADCAST_MODE,
  NTP_CONTROL_MODE,
  NUM_NTP_MODES
};

enum NTPLeapIndicator { NTP_LI_NONE, NTP_LI_LAST_MINUTE_OF_THE_DAY_61_S, NTP_LI_LAST_MINUTE_OF_THE_DAY_59_S, NTP_LI_NOSYNC };

time_t ntpToUnixTime(NTPTime ntpTime);
uint32_t unixToNTPTime(time_t unixTime);

void convertNTPTimestamp(NTPTime &timestamp);
void dumpNTPPacketData(const NTPPacket packet);

#endif
