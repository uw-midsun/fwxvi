#ifndef I2C_DATAGRAM_H
#define I2C_DATAGRAM_H

#include <string>

#include "command_code.h"

namespace Datagram {
class I2C {
 public:
  static constexpr size_t MAX_BUFFER_SIZE = 256;

  enum class Port { I2C_PORT_1, I2C_PORT_2, NUM_I2C_PORTS };

  struct Payload {
    Port i2cPort;
    size_t bufferLength;
    uint8_t buffer[MAX_BUFFER_SIZE];
  };

  explicit I2C(Payload &data);
  I2C() = default;

  std::string serialize(const CommandCode &commandCode) const;
  void deserialize(std::string &i2cDatagramPayload);

  void setI2CPort(const Port &i2cPort);
  void setBuffer(const uint8_t *data, size_t length);

  void clearBuffer();

  Port getI2CPort() const;
  size_t getBufferLength() const;
  const uint8_t *getBuffer() const;

 private:
  Payload m_i2cDatagram;
};

}  // namespace Datagram
#endif
