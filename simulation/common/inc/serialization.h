#pragma once

/************************************************************************************************
 * @file   serialization.h
 *
 * @brief  Header file defining the serialization helper functions
 *
 * @date   2025-01-04
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <cstring>
#include <string>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup SerializationHelpers
 * @brief    Serialization helper library
 * @{
 */

/**
 * @brief   Serialize an integer value
 * @details The integer will be converted to a string of size T.
 *          Size T is dependent on the type of integer.
 *          The stringified-integer is appended to the target.
 * @param   target Existing message payload
 * @param   value Value to be appended
 */
template <typename T>
void serializeInteger(std::string &target, T value) {
  target.append(reinterpret_cast<const char *>(&value), sizeof(T));
}

/**
 * @brief   Serialize a string value
 * @details The string will be pre-fixed with a 16-bit size to indicate the size
 *          The string is appended to the target. The final packet follows as such:
 *          | target | 16-bit length | str |
 * @param   target Existing message payload
 * @param   str String to be appended
 */
void serializeString(std::string &target, const std::string &str);

/**
 * @brief   Deserialize an integer value
 * @details The offset will automatically be incremented in this function
 * @param   source Message payload to be decoded
 * @param   offset Byte offset from the start of the message payload
 * @return  Deserialized integer value
 */
template <typename T>
T deserializeInteger(const std::string &source, size_t &offset) {
  T value;
  std::memcpy(&value, source.data() + offset, sizeof(T));
  offset += sizeof(T);
  return value;
}

/**
 * @brief   Deserialize a string value
 * @details This will fetch the string value based on the stored length
 *          The offset will automatically be incremented in this function
 * @param   source Message payload to be decoded
 * @param   offset Byte offset from the start of the message payload
 * @return  Deserialized string value
 */
std::string deserializeString(std::string &source, size_t &offset);

/** @} */
