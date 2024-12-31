#ifndef SERIALIZATION
#define SERIALIZATION

#include <cstring>
#include <string>

/**
 * @brief   Serialize an integer value
 * @details The integer will be converted to a string of size T.
 *          Size T is dependent on the type of integer.
 *          The stringified-integer is appended to the target.
 * @param   target
 * @param   value
 */
template <typename T>
void serializeInteger(std::string &target, T value) {
  target.append(reinterpret_cast<const char *>(&value), sizeof(T));
}

/**
 * @brief   Serialize a string value
 * @details The string will be pre-fixed with a 16-bit size to indicate the
 * size. The string is appended to the target. The final packet follows as such:
 *          | target | 16-bit length | str |
 * @param   target
 * @param   str
 */
void serializeString(std::string &target, const std::string &str);

template <typename T>
T deserializeInteger(const std::string &source, size_t &offset) {
  T value;
  std::memcpy(&value, source.data() + offset, sizeof(T));
  offset += sizeof(T);
  return value;
}

std::string deserializeString(std::string &source, size_t &offset);

#endif
