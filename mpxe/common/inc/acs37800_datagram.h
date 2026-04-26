#pragma once

/************************************************************************************************
 * @file   acs37800_datagram.h
 *
 * @brief  Header file defining the acs37800
 *
 * @date   2026-03-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <string>

/* Intra-component Headers */
#include "command_code.h"

namespace Datagram {

/**
 * @class   Datagram::Acs37800
 * @brief   Class for managing stored values in Acs37800
 * @details This class provides an interface for simulating values in the Acs37800.
 */
class Acs37800 {
 public:
  static constexpr uint8_t MASK_FAULTOUT = 0x02;     /* Bit 1 */
  static constexpr uint8_t MASK_OVERVOLTAGE = 0x08;  /* Bit 3 */
  static constexpr uint8_t MASK_UNDERVOLTAGE = 0x10; /* Bit 4 */

  /**
   * @brief Acs37800 Datagram payload storage
   */
  struct Payload {
    float voltage_mV;
    float current_amps;
    float active_power_mW;
    bool overcurrent_flag;
    bool overvoltage_flag;
    bool undervoltage_flag;
  };

  /**
   * @brief Constructs a Acs37800 object with provided payload data
   */
  explicit Acs37800(Payload &data);

  /**
   * @brief Default constructor for Acs37800 object
   */
  Acs37800() = default;

  /**
   * @brief   Serializes acs37800 data with command code for transmission
   * @param   commandCode Command code to include in serialized data
   * @return  Serialized string containing acs37800 data
   */
  std::string serialize(const CommandCode &commandCode) const;

  /**
   * @brief Deserializes ltc acs37800 data from payload string
   * @param acs37800DatagramPayload String containing serialized Ltc Acs37800 data
   */
  void deserialize(std::string &acs37800DatagramPayload);

  /**
   * @brief   Sets voltage
   * @param   voltage_mV Voltage to set as target
   */
  void setVoltage(float voltage_mV);

  /**
   * @brief   Sets current
   * @param   current_amps Current to set as target
   */
  void setCurrent(float current_amps);

  /**
   * @brief   Sets active power
   * @param   active_power_mW Active power to set as target
   */
  void setActivePower(float active_power_mW);

  /**
   * @brief   Sets overcurrent flag
   * @param   overcurrent_flag Overcurrent flag to set as target
   */
  void setOvercurrentFlag(bool overcurrent_flag);

  /**
   * @brief   Sets overvoltage flag
   * @param   overvoltage_flag Overvoltage flag to set as target
   */
  void setOvervoltageFlag(bool overvoltage_flag);

  /**
   * @brief   Sets undervoltage flag
   * @param   undervoltage_flag Undervoltage flag to set as target
   */
  void setUndervoltageFlag(bool undervoltage_flag);

  /**
   * @brief   Gets voltage in mV
   * @return  voltage in mV
   */
  float getVoltage() const;

  /**
   * @brief   Gets current in amps
   * @return  current in amps
   */
  float getCurrent() const;

  /**
   * @brief   Gets active power in mW
   * @return  active power in mW
   */
  float getActivePower() const;

  /**
   * @brief   Gets overcurrent flag
   * @return  Overcurrent flag
   */
  bool getOvercurrentFlag() const;

  /**
   * @brief   Gets overvoltage flag
   * @return  Overvoltage flag
   */
  bool getOvervoltageFlag() const;

  /**
   * @brief   Gets undervoltage flag
   * @return  Undervoltage flag
   */
  bool getUndervoltageFlag() const;

 private:
  Payload m_acs37800Datagram;
};

}  // namespace Datagram

/** @} */
