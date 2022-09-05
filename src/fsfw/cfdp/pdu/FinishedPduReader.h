#ifndef FSFW_CFDP_PDU_FINISHEDPDUDESERIALIZER_H_
#define FSFW_CFDP_PDU_FINISHEDPDUDESERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveReader.h"
#include "fsfw/cfdp/pdu/FinishedInfo.h"

class FinishPduReader : public FileDirectiveReader {
 public:
  FinishPduReader(const uint8_t* pduBuf, size_t maxSize, FinishedInfo& info);

  ReturnValue_t parseData() override;

  FinishedInfo& getInfo();

 private:
  FinishedInfo& finishedInfo;

  ReturnValue_t parseTlvs(size_t remLen, size_t currentIdx, const uint8_t* buf,
                          cfdp::ConditionCode conditionCode);
};

#endif /* FSFW_CFDP_PDU_FINISHEDPDUDESERIALIZER_H_ */
