#ifndef FSFW_CFDP_PDU_KEEPALIVEPDUSERIALIZER_H_
#define FSFW_CFDP_PDU_KEEPALIVEPDUSERIALIZER_H_

#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"

class KeepAlivePduCreator : public FileDirectiveCreator {
 public:
  KeepAlivePduCreator(PduConfig& conf, cfdp::FileSize& progress);

  void updateDirectiveFieldLen();

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

 private:
  cfdp::FileSize& progress;
};

#endif /* FSFW_CFDP_PDU_KEEPALIVEPDUSERIALIZER_H_ */
