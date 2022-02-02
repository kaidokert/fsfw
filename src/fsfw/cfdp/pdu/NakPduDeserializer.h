#ifndef FSFW_SRC_FSFW_CFDP_PDU_NAKPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_NAKPDUDESERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"
#include "fsfw/cfdp/pdu/NakInfo.h"

class NakPduDeserializer : public FileDirectiveDeserializer {
 public:
  NakPduDeserializer(const uint8_t* pduBuf, size_t maxSize, NakInfo& info);

  /**
   * This needs to be called before accessing the PDU fields to avoid segmentation faults.
   * @return
   */
  virtual ReturnValue_t parseData() override;

 private:
  NakInfo& nakInfo;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_NAKPDUDESERIALIZER_H_ */
