#ifndef FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUDESERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveReader.h"

class PromptPduReader : public FileDirectiveReader {
 public:
  PromptPduReader(const uint8_t *pduBuf, size_t maxSize);

  [[nodiscard]] cfdp::PromptResponseRequired getPromptResponseRequired() const;
  ReturnValue_t parseData() override;

 private:
  cfdp::PromptResponseRequired responseRequired = cfdp::PromptResponseRequired::PROMPT_NAK;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUDESERIALIZER_H_ */
