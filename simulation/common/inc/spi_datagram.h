#ifndef SPI_DATAGRAM_H
#define SPI_DATAGRAM_H

#include <string>

#include "command_code.h"

namespace Datagram {
class SPI {
 public:
  static constexpr size_t SPI_MAX_BUFFER_SIZE = 256;

  enum class Port { SPI_PORT_1, SPI_PORT_2, NUM_SPI_PORTS };

  struct Payload {
    Port spiPort;
    size_t bufferLength;
    uint8_t buffer[SPI_MAX_BUFFER_SIZE];
  };

  explicit SPI(Payload &data);
  SPI() = default;

  std::string serialize(const CommandCode &commandCode) const;
  void deserialize(std::string &spiDatagramPayload);

  void setSPIPort(const Port &spiPort);
  void setBuffer(const uint8_t *data, size_t length);

  void clearBuffer();

  Port getSPIPort() const;
  size_t getBufferLength() const;
  const uint8_t *getBuffer() const;

 private:
  Payload m_spiDatagram;
};

}  // namespace Datagram
#endif
