#ifndef FSFW_SRC_FSFW_CFDP_PDU_KEEPALIVEPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_KEEPALIVEPDUSERIALIZER_H_

#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/pdu/FileDirectiveSerializer.h"

class KeepAlivePduSerializer : public FileDirectiveSerializer {
 public:
  KeepAlivePduSerializer(PduConfig& conf, cfdp::FileSize& progress);

  void updateDirectiveFieldLen();

  size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

 private:
  cfdp::FileSize& progress;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_KEEPALIVEPDUSERIALIZER_H_ */
