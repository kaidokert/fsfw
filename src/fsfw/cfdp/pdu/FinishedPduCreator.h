#ifndef FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_
#define FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_

#include "FinishedInfo.h"
#include "fsfw/cfdp/pdu/FileDataCreator.h"
#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"

class FinishPduCreator : public FileDirectiveCreator {
 public:
  FinishPduCreator(PduConfig& pduConf, FinishedInfo& finishInfo);

  void updateDirectiveFieldLen();

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;
  using FileDirectiveCreator::serialize;

 private:
  FinishedInfo& finishInfo;
};

#endif /* FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_ */
