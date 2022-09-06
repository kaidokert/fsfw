#ifndef FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_

#include "EofInfo.h"
#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"
#include "fsfw/cfdp/tlv/EntityIdTlv.h"

class EofPduCreator : public FileDirectiveCreator {
 public:
  EofPduCreator(PduConfig& conf, EofInfo& info);

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  using FileDirectiveCreator::serialize;

 private:
  EofInfo& info;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_ */
