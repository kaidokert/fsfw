#ifndef FSFW_SRC_FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_

#include "FinishedInfo.h"
#include "fsfw/cfdp/pdu/FileDataCreator.h"
#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"

class FinishPduSerializer : public FileDirectiveCreator {
 public:
  FinishPduSerializer(PduConfig& pduConf, FinishedInfo& finishInfo);

  void updateDirectiveFieldLen();

  size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

 private:
  FinishedInfo& finishInfo;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_FINISHEDPDUSERIALIZER_H_ */
