#ifndef FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_

#include "EofInfo.h"
#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"
#include "fsfw/cfdp/tlv/EntityIdTlv.h"

class EofPduSerializer : public FileDirectiveCreator {
 public:
  EofPduSerializer(PduConfig& conf, EofInfo& info);

  size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

 private:
  EofInfo& info;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_EOFPDUSERIALIZER_H_ */
