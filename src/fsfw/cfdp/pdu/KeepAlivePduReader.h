#ifndef FSFW_CFDP_PDU_KEEPALIVEREADER_H_
#define FSFW_CFDP_PDU_KEEPALIVEREADER_H_

#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/pdu/FileDirectiveReader.h"

class KeepAlivePduReader : public FileDirectiveReader {
 public:
  KeepAlivePduReader(const uint8_t* pduBuf, size_t maxSize, cfdp::FileSize& progress);

  ReturnValue_t parseData() override;

  cfdp::FileSize& getProgress();

 private:
  cfdp::FileSize& progress;
};

#endif /* FSFW_CFDP_PDU_KEEPALIVEPDUREADER_H_ */
