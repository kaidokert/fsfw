#ifndef FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUSERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUSERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"

class PromptPduSerializer : public FileDirectiveCreator {
 public:
  PromptPduSerializer(PduConfig& conf, cfdp::PromptResponseRequired responseRequired);

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

 private:
  cfdp::PromptResponseRequired responseRequired;
};

#endif /* FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUSERIALIZER_H_ */
