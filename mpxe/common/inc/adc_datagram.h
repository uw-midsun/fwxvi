#pragma once

/************************************************************************************************
 * @file   adc_datagram.h
 *
 * @brief  Header file defining the AdcDatagram class
 *
 * @date   2025-03-28
 * @author Shahzaib Siddiqui
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <string>

/* Inter-component Headers */

/* Intra-component Headers */
#include "command_code.h"

/**
 * @defgroup AdcDatagram
 * @brief    Shared Adc Datagram class
 * @{
 */

namespace Datagram {

/**
 * @class   DatagramAdc
 * @brief
 * @details
 */
class Adc {
 public:
  static const constexpr uint8_t MPXE_NUM_ADC_CHANNELS = 19U; /**< Total Number of ADC Channels */

  /**
   * @brief   GPIO Port definition
   */
  enum class Port {
    GPIO_PORT_A = 0, /**< GPIO port A */
    GPIO_PORT_B,     /**< GPIO port B */
    NUM_GPIO_PORTS   /**< Number of GPIO Ports */
  };

  /** Maximum buffer size dependent on the Max number of ADC channels */
  static constexpr size_t ADC_MAX_BUFFER_SIZE = MPXE_NUM_ADC_CHANNELS * 2U;

  /**
   * @brief   Adc Datagram payload storage
   */
  struct Payload {
    Port gpioPort;                       /**< Gpio port used for ADC*/
    uint8_t gpioPin;                     /**< Gpio pin used for ADC*/
    uint8_t bufferLength;                /**< Data buffer length */
    uint8_t buffer[ADC_MAX_BUFFER_SIZE]; /**< Data buffer */
  };

  /**
   * @brief   Constructs an Adc object with provided payload data
   * @param   data Reference to payload data
   */
  explicit Adc(Payload &data);

  /**
   * @brief   Default constructor for Adc object
   */
  Adc() = default;

  /**
   * @brief   Serializes Adc data with command code for transmission
   * @param   commandCode Command code to include in serialized data
   * @return  Serialized string containing Adc data
   */
  std::string serialize(const CommandCode &commandCode) const;

  /**
   * @brief   Deserializes Adc data from payload string
   * @param   gpioDatagramPayload String containing serialized Gpio data
   */
  void deserialize(std::string &AdcDatagramPayload);

  /**
   * @brief   Sets the target Gpio port for Adc
   * @param   gpioPort Port to set as target
   */
  void setGpioPort(const Port &gpioPort);

  /**
   * @brief   Sets the target Gpio pin for Adc
   * @param   gpioPin Pin to set as target
   */
  void setGpioPin(const uint8_t &gpioPin);

  /**
   * @brief   Sets data in the Adc buffer
   * @param   data Pointer to data to copy into buffer
   * @param   length Length of data to copy
   */
  void setBuffer(const uint8_t *data, uint8_t length);

  /**
   * @brief   Clear the Adc data buffer
   */
  void clearBuffer();

  /**
   * @brief   Gets the target Gpio port
   * @return  Target port
   */
  Port getGpioPort() const;

  /**
   * @brief   Gets the target Gpio pin
   * @return  Target pin
   */
  uint8_t getGpioPin() const;

  /**
   * @brief   Gets the Adc buffer length
   * @return  Buffer length
   */
  uint8_t getBufferLength() const;

  /**
   * @brief   Gets the Adc buffer
   * @return  Pointer to the buffer array
   */
  const uint8_t *getBuffer() const;

 private:
  Payload m_adcDatagram; /**< Private datagram payload */
};

}  // namespace Datagram

/** @} */
