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
  /**
   * @brief Acs37800 Datagram payload storage
   */
  struct Payload {
    // TODO
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


  // TODO Setters and Getters for Payload

 private:
  Payload m_acs37800Datagram;
};

}  // namespace Datagram

/** @} */
