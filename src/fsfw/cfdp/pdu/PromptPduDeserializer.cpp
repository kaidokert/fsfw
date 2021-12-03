#include "PromptPduDeserializer.h"

PromptPduDeserializer::PromptPduDeserializer(const uint8_t *pduBuf, size_t maxSize):
        FileDirectiveDeserializer(pduBuf, maxSize) {
}

cfdp::PromptResponseRequired PromptPduDeserializer::getPromptResponseRequired() const {
    return responseRequired;
}

ReturnValue_t PromptPduDeserializer::parseData() {
    ReturnValue_t result = FileDirectiveDeserializer::parseData();
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    size_t currentIdx = FileDirectiveDeserializer::getHeaderSize();
    if (FileDirectiveDeserializer::getWholePduSize() - currentIdx < 1) {
        return SerializeIF::STREAM_TOO_SHORT;
    }
    responseRequired = static_cast<cfdp::PromptResponseRequired>((rawPtr[currentIdx] >> 7) & 0x01);
    return HasReturnvaluesIF::RETURN_OK;
}
