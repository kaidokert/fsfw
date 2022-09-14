#ifndef FSFW_CFDP_PDU_NAKPDUDESERIALIZER_H_
#define FSFW_CFDP_PDU_NAKPDUDESERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveReader.h"
#include "fsfw/cfdp/pdu/NakInfo.h"

class NakPduReader : public FileDirectiveReader {
 public:
  NakPduReader(const uint8_t* pduBuf, size_t maxSize, NakInfo& info);

  /**
   * This needs to be called before accessing the PDU fields to avoid segmentation faults.
   * @return
   */
  ReturnValue_t parseData() override;

 private:
  NakInfo& nakInfo;
};

#endif /* FSFW_CFDP_PDU_NAKPDUDESERIALIZER_H_ */
