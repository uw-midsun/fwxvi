#pragma once

/************************************************************************************************
 * @file    adc_manager.h
 *
 * @brief   Adc Manager
 *
 * @date    2025-09-27
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/
/* Standard library Headers */
#include <stdint.h>
#include <string.h>

#include <unordered_map>

/* Inter-component Headers */
#include "adc_datagram.h"

/* Intra-component Headers */

/**
 * @defgroup ServerAdcManager
 * @brief    AdcManager for the server
 * @{
 */

/**
 * @class AdcManager
 * @brief   Class that manages receiving and transmitting ADC commands and JSON logging
 * @details This class is responsible for transmitting serialized messages for reading/writing ADC values. 
 *          It shall support reading/writing to all channels or individual channels
 *          The class shall cache current ADC data using a hash-map.
 */
class AdcManager {
 private:
  /** @brief Hash map to store GPIO Address and reading for each channel */
  using AdcChannelInfo = std::unordered_map<std::string, std::string>;
  /** @brief Hash map to store ADC data keyed by Channel */
  using AdcReadingInfo = std::unordered_map<std::string, AdcChannelInfo>;
  std::unordered_map<std::string, AdcReadingInfo> m_adcInfo; /**< Hash-map to cache all ADC data */

  using GpioPort = Datagram::Adc::Port; /**< An Alias for GPIO Port */
  Datagram::Adc m_adcDatagram;          /**< Datagram object to serialize/deserialize payloads */

  /**
   * @brief  Struct representing a GPIO port/pin mapped to an ADC channel
   */
  struct Mapping {
    GpioPort port;   /**< GPIO Port */
    uint8_t pin;     /**< GPIO Pin */
    uint8_t channel; /**< ADC Channel */
  };

  /** @brief Lookup table mapping ports/pins to ADC channels */
  static constexpr Mapping channelMap[] = { { GpioPort::GPIO_PORT_C, 0, 1 },  { GpioPort::GPIO_PORT_C, 1, 2 },  { GpioPort::GPIO_PORT_C, 2, 3 },  { GpioPort::GPIO_PORT_C, 3, 4 },
                                            { GpioPort::GPIO_PORT_A, 0, 5 },  { GpioPort::GPIO_PORT_A, 1, 6 },  { GpioPort::GPIO_PORT_A, 2, 7 },  { GpioPort::GPIO_PORT_A, 3, 8 },
                                            { GpioPort::GPIO_PORT_A, 4, 9 },  { GpioPort::GPIO_PORT_A, 5, 10 }, { GpioPort::GPIO_PORT_A, 6, 11 }, { GpioPort::GPIO_PORT_A, 7, 12 },
                                            { GpioPort::GPIO_PORT_C, 4, 13 }, { GpioPort::GPIO_PORT_C, 5, 14 }, { GpioPort::GPIO_PORT_B, 0, 15 }, { GpioPort::GPIO_PORT_B, 1, 16 } };

  /**
   * @brief   Save cached ADC readings for a project to JSON storage
   * @param   projectName Name of the project whose ADC info should be saved
   */
  void loadAdcInfo(std::string &projectName);

  /**
   * @brief   Save cached ADC readings for a project to JSON storage
   * @param   projectName Name of the project whose ADC info should be saved
   */
  void saveAdcInfo(std::string &projectName);

  /**
   * @brief   Convert GPIO Address to string
   * @param   port GPIO Port
   * @param   pin GPIO Pin
   * @return  std::string String representation of GPIO Address
   */
  std::string stringifyGpioAddress(GpioPort port, uint8_t pin);

  /**
   * @brief   Convert a raw ADC data buffer into a string
   * @details If multiple readings are present, they are separated by commas in the string.
   *          Currently, only a single reading is stored per channel, so this behavior is not
   *          used. However, it is included for potential future support of multiple readings
   *          per channel.
   * @param   port GPIO port
   * @param   pin GPIO pin
   * @return  std::string String representation of the buffer
   */
  std::string stringifyReadings(const uint8_t *&buffer, size_t bufferLength);

  /**
   * @brief   Get the Adc Channel as a string
   * @param   port GPIO port
   * @param   pin GPIO pin
   * @return  std::string Adc Channel as a string
   */
  std::string getChannel(GpioPort port, uint8_t pin);

 public:
  /**
   * @brief   Construct a new Adc Manager object
   * @details Default constructor
   */
  AdcManager() = default;

  /**
   * @brief   Update a raw ADC reading for a specific ADC Channel
   * @param   projectName Project name for which data is updated
   * @param   payload Serialized data payload containing the reading
   */
  void updateAdcRaw(std::string &projectName, std::string &payload);

  /**
   * @brief   Update raw ADC readings for all ADC Channels
   * @param   projectName Project name for which data is updated
   * @param   payload Serialized data payload containing all raw readings
   */
  void updateAdcRawAll(std::string &projectName, std::string &payload);

  /**
   * @brief   Update a converted ADC reading for a specific ADC Channel
   * @param   projectName Project name for which data is updated
   * @param   payload Serialized data payload containing all raw readings
   */
  void updateAdcConverted(std::string &projectName, std::string &payload);

  /**
   * @brief   Update a converted ADC reading for a all ADC Channel
   * @param   projectName Project name for which data is updated
   * @param   payload Serialized data payload containing all raw readings
   */
  void updateAdcConvertedAll(std::string &projectName, std::string &payload);

  /**
   * @brief Create an ADC command for raw or converted readings
   * @details This function supports both GET and SET commands for individual or all channels
   * @param commandCode Command reference to be transmitted to the client
   * @param gpioAddress GPIO address (e.g., "A0", "C3") for channel-specific commands
   * @param readings Data payload for SET commands (ignored for GET commands)
   * @return Fully serialized data payload to be transmitted to the client
   */
  std::string createAdcCommand(CommandCode commandCode, std::string gpioAddress, std::string readings);
};

/** @} */
