/************************************************************************************************
 * @file   acs37800_datagram.cc
 *
 * @brief  Source file defining the acs37800 datagram class
 *
 * @date   2026-03-08
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

/* Inter-component Headers */

/* Intra-component Headers */
#include "acs37800_datagram.h"
#include "serialization.h"

namespace Datagram {

Acs37800::Acs37800(Payload &data) {
  m_acs37800Datagram = data;
}

std::string Acs37800::serialize(const CommandCode &commandCode) const {
  std::string serializedData;

  // TODO

  return encodeCommand(commandCode, serializedData);
}

void Acs37800::deserialize(std::string &acs37800DatagramPayload) {
  // TODO
}

}  // namespace Datagram

// TODO Setters and Getters for Payload
