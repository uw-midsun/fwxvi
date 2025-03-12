/************************************************************************************************
 * @file   serialization.cc
 *
 * @brief  Source file defining the Serialization helpers
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <iostream>

/* Inter-component Headers */

/* Intra-component Headers */
#include "serialization.h"

void serializeString(std::string &target, const std::string &str) {
  uint16_t length = static_cast<uint16_t>(str.length());
  serializeInteger(target, length);
  target.append(str);
}

std::string deserializeString(std::string &source, size_t &offset) {
  uint16_t length = deserializeInteger<uint16_t>(source, offset);
  std::string str = source.substr(offset, length);
  offset += length;
  return str;
}
