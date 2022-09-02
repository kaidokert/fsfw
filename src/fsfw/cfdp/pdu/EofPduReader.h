#ifndef FSFW_SRC_FSFW_CFDP_PDU_EOFPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_EOFPDUDESERIALIZER_H_

#include "EofInfo.h"
#include "fsfw/cfdp/pdu/FileDirectiveReader.h"

class EofPduReader : public FileDirectiveReader {
 public:
  EofPduReader(const uint8_t* pduBuf, size_t maxSize, EofInfo& eofInfo);

  ReturnValue_t parseData() override;

 private:
  EofInfo& info;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_EOFPDUDESERIALIZER_H_ */
