#ifndef FSFW_SRC_FSFW_CFDP_PDU_KEEPALIVEPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_KEEPALIVEPDUDESERIALIZER_H_

#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"

class KeepAlivePduDeserializer : public FileDirectiveDeserializer {
 public:
  KeepAlivePduDeserializer(const uint8_t* pduBuf, size_t maxSize, cfdp::FileSize& progress);

  ReturnValue_t parseData() override;

  cfdp::FileSize& getProgress();

 private:
  cfdp::FileSize& progress;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_KEEPALIVEPDUDESERIALIZER_H_ */
