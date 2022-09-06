#ifndef FSFW_CFDP_PDU_FILEDATASERIALIZER_H_
#define FSFW_CFDP_PDU_FILEDATASERIALIZER_H_

#include "../definitions.h"
#include "FileDataInfo.h"
#include "HeaderCreator.h"

class FileDataCreator : public HeaderCreator {
 public:
  FileDataCreator(PduConfig& conf, FileDataInfo& info);

  void update();

  ReturnValue_t serialize(uint8_t* buf, size_t& serLen, size_t maxSize) const {
    return SerializeIF::serialize(buf, serLen, maxSize, SerializeIF::Endianness::NETWORK);
  }

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

 private:
  FileDataInfo& info;
};

#endif /* FSFW_CFDP_PDU_FILEDATASERIALIZER_H_ */
