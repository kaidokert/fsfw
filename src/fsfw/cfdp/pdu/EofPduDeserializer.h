#ifndef FSFW_SRC_FSFW_CFDP_PDU_EOFPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_EOFPDUDESERIALIZER_H_

#include "EofInfo.h"
#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"

class EofPduDeserializer : public FileDirectiveDeserializer {
 public:
  EofPduDeserializer(const uint8_t* pduBuf, size_t maxSize, EofInfo& eofInfo);

  virtual ReturnValue_t parseData() override;

 private:
  EofInfo& info;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_EOFPDUDESERIALIZER_H_ */
