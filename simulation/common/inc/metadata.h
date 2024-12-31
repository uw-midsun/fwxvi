#ifndef METADATA_H
#define METADATA_H

#include <cstdint>
#include <string>

namespace Datagram {
class Metadata {
 public:
  struct Payload {
    std::string projectName;
    std::string projectStatus;
    std::string hardwareModel;
  };
  explicit Metadata(Payload &data);
  Metadata() = default;

  std::string serialize() const;
  void deserialize(std::string &metadataPayload);

  void setProjectName(const std::string &projectName);
  void setProjectStatus(const std::string &projectStatus);
  void setHardwareModel(const std::string &hardwareModel);

  std::string getProjectName() const;
  std::string getProjectStatus() const;
  std::string getHardwareModel() const;

 private:
  Payload m_metadata;
};

}  // namespace Datagram
#endif
