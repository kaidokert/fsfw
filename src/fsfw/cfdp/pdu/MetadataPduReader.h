#ifndef FSFW_CFDP_PDU_METADATAPDUREADER_H_
#define FSFW_CFDP_PDU_METADATAPDUREADER_H_

#include "fsfw/cfdp/pdu/FileDirectiveReader.h"
#include "fsfw/cfdp/pdu/MetadataInfo.h"

class MetadataPduReader : public FileDirectiveReader {
 public:
  MetadataPduReader(const uint8_t* pduBuf, size_t maxSize, MetadataInfo& info);

  ReturnValue_t parseData() override;

 private:
  MetadataInfo& info;
};

#endif /* FSFW_CFDP_PDU_METADATAPDUREADER_H_ */
