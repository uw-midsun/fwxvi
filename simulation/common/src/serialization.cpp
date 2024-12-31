#include "serialization.h"

#include <iostream>
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
