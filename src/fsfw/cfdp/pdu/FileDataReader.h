#ifndef FSFW_SRC_FSFW_CFDP_PDU_FILEDATADESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FILEDATADESERIALIZER_H_

#include "../definitions.h"
#include "FileDataInfo.h"
#include "PduHeaderReader.h"

class FileDataReader : public PduHeaderReader {
 public:
  FileDataReader(const uint8_t* pduBuf, size_t maxSize, FileDataInfo& info);

  ReturnValue_t parseData() override;
  [[nodiscard]] SerializeIF::Endianness getEndianness() const;
  void setEndianness(SerializeIF::Endianness endianness = SerializeIF::Endianness::NETWORK);

 private:
  SerializeIF::Endianness endianness = SerializeIF::Endianness::NETWORK;
  FileDataInfo& info;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FILEDATADESERIALIZER_H_ */
