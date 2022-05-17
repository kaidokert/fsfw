#ifndef FSFW_SRC_FSFW_CFDP_PDU_FILEDATASERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FILEDATASERIALIZER_H_

#include "../definitions.h"
#include "FileDataInfo.h"
#include "HeaderSerializer.h"

class FileDataSerializer : public HeaderSerializer {
 public:
  FileDataSerializer(PduConfig& conf, FileDataInfo& info);

  void update();

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  size_t getSerializedSize() const override;

 private:
  FileDataInfo& info;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FILEDATADESERIALIZER_H_ */
