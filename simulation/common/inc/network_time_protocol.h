#pragma once

/************************************************************************************************
 * @file   network_time_protocol.h
 *
 * @brief  Header file defining the Network time protocol helpers
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <ctime>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup NetworkTimeProtocolHelpers
 * @brief    Network time protocol helper library
 * @{
 */

/**
 * @brief   NTP version being used
 */
#define NTP_VERSION 4U

/**
 * @brief   Offset for the Leap Indicator in the flags field of the NTP packet
 */
#define NTP_LEAP_INDICATOR_OFFSET 6U

/**
 * @brief   Offset for the NTP version in the flags field of the NTP packet
 */
#define NTP_VERSION_OFFSET 3U

/**
 * @brief   Offset for the NTP mode in the flags field of the NTP packet
 */
#define NTP_MODE_OFFSET 0U

/**
 * @brief   Macro to convert NTP poll interval to seconds
 * @param   poll The poll interval exponent
 */
#define NTP_POLL_TO_SECONDS(poll) (1U << poll)

/**
 * @brief   Difference between the NTP epoch (1900-01-01) and the Unix epoch (1970-01-01) in seconds
 */
static constexpr uint32_t NTP_UNIX_EPOCH_DIFF = 2208988800UL;

/**
 * @brief   Structure representing NTP time format with seconds and fractional seconds
 */
struct NTPTime {
  uint32_t seconds;  /**< Seconds since NTP epoch */
  uint32_t fraction; /**< Fractional seconds (1 / 2^32 of a second). */
};

/**
 * @brief   Structure representing an NTP packet
 */
struct NTPPacket {
  uint8_t flags;           /**< Flags field containing Leap Indicator, Version, and Mode */
  uint8_t stratum;         /**< Stratum level of the server */
  uint8_t poll;            /**< Maximum interval between successive messages, in log2 seconds */
  uint8_t precision;       /**< Precision of the local clock, in log2 seconds */
  uint32_t rootDelay;      /**< Total round-trip delay to the primary reference source */
  uint32_t rootDispersion; /**< Maximum error relative to the primary reference source */
  uint8_t referenceId[4];  /**< Identifier of the reference clock */
  NTPTime referenceTime;   /**< Time when the system clock was last set or corrected */
  NTPTime originTime;      /**< Time at the client when the request departed */
  NTPTime receiveTime;     /**< Time at the server when the request was received */
  NTPTime transmitTime;    /**< Time at the server when the response was sent */
};

/**
 * @brief   Enumeration for the NTP modes
 */
enum NTPMode {
  NTP_RESERVED_MODE,  /**< Reserved mode (not used) */
  NTP_ACTIVE_MODE,    /**< Symmetric active mode */
  NTP_PASSIVE_MODE,   /**< Symmetric passive mode */
  NTP_CLIENT_MODE,    /**< Client mode */
  NTP_SERVER_MODE,    /**< Server mode */
  NTP_BROADCAST_MODE, /**< Broadcast mode */
  NTP_CONTROL_MODE,   /**< Control mode */
  NUM_NTP_MODES       /**< Number of modes */
};

/**
 * @brief   Enumeration for the Leap Indicator values
 */
enum NTPLeapIndicator {
  NTP_LI_NONE,                        /**< No leap second adjustment */
  NTP_LI_LAST_MINUTE_OF_THE_DAY_61_S, /**< Last minute of the day has 61 seconds */
  NTP_LI_LAST_MINUTE_OF_THE_DAY_59_S, /**< Last minute of the day has 59 seconds */
  NTP_LI_NOSYNC                       /**< Clock is not synchronized */
};

/**
 * @brief   Converts an NTP time to Unix time
 * @param   ntpTime The NTP time to be converted
 * @return  The corresponding Unix time
 */
time_t ntpToUnixTime(NTPTime ntpTime);

/**
 * @brief   Converts a Unix time to NTP time
 * @param   unixTime The Unix time to be converted
 * @return  The corresponding NTP time in seconds since the NTP epoch
 */
uint32_t unixToNTPTime(time_t unixTime);

/**
 * @brief   Converts an NTP timestamp to its human-readable representation
 * @param   timestamp The NTP timestamp to be converted
 */
void convertNTPTimestamp(NTPTime &timestamp);

/**
 * @brief   Dumps the data of an NTP packet for debugging purposes
 * @param   packet The NTP packet to be dumped
 */
void dumpNTPPacketData(const NTPPacket packet);

/** @} */
