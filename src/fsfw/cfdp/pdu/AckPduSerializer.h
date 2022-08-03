#ifndef FSFW_SRC_FSFW_CFDP_PDU_ACKPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_ACKPDUSERIALIZER_H_

#include "AckInfo.h"
#include "FileDirectiveCreator.h"
#include "FileDirectiveReader.h"

class AckPduSerializer : public FileDirectiveCreator {
 public:
  /**
   * @brief   Serializer to pack ACK PDUs
   * @details
   * Please note that only Finished PDUs and EOF are acknowledged.
   * @param ackedDirective
   * @param ackedConditionCode
   * @param transactionStatus
   * @param pduConf
   */
  AckPduSerializer(AckInfo& ackInfo, PduConfig& pduConf);

  size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

 private:
  AckInfo& ackInfo;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_ACKPDUSERIALIZER_H_ */
