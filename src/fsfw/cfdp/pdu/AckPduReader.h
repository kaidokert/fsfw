#ifndef FSFW_SRC_FSFW_CFDP_PDU_ACKPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_ACKPDUDESERIALIZER_H_

#include "AckInfo.h"
#include "fsfw/cfdp/pdu/FileDirectiveReader.h"

class AckPduReader : public FileDirectiveReader {
 public:
  AckPduReader(const uint8_t* pduBuf, size_t maxSize, AckInfo& info);

  /**
   *
   * @return
   *  - cfdp::INVALID_DIRECTIVE_FIELDS: Invalid fields
   */
  ReturnValue_t parseData() override;

  static bool checkAckedDirectiveField(uint8_t firstPduDataByte,
                                       cfdp::FileDirective& ackedDirective);

 private:
  bool checkAndSetCodes(uint8_t rawAckedByte, uint8_t rawAckedConditionCode);
  AckInfo& info;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_ACKPDUDESERIALIZER_H_ */
