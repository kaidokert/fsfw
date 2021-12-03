#ifndef FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUDESERIALIZER_H_
#define FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUDESERIALIZER_H_

#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"

class PromptPduDeserializer: public FileDirectiveDeserializer {
public:
    PromptPduDeserializer(const uint8_t *pduBuf, size_t maxSize);

    cfdp::PromptResponseRequired getPromptResponseRequired() const;
    ReturnValue_t parseData() override;
private:
    cfdp::PromptResponseRequired responseRequired = cfdp::PromptResponseRequired::PROMPT_NAK;
};



#endif /* FSFW_SRC_FSFW_CFDP_PDU_PROMPTPDUDESERIALIZER_H_ */
