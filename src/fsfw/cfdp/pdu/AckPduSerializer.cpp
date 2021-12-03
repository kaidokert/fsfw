#include "AckPduSerializer.h"

AckPduSerializer::AckPduSerializer(AckInfo& ackInfo, PduConfig &pduConf):
        FileDirectiveSerializer(pduConf, cfdp::FileDirectives::ACK, 2),
        ackInfo(ackInfo) {
}

size_t AckPduSerializer::getSerializedSize() const {
    return FileDirectiveSerializer::getWholePduSize();
}

ReturnValue_t AckPduSerializer::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
        Endianness streamEndianness) const {
    ReturnValue_t result = FileDirectiveSerializer::serialize(buffer, size, maxSize,
            streamEndianness);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    cfdp::FileDirectives ackedDirective = ackInfo.getAckedDirective();
    uint8_t directiveSubtypeCode = ackInfo.getDirectiveSubtypeCode();
    cfdp::ConditionCode ackedConditionCode = ackInfo.getAckedConditionCode();
    cfdp::AckTransactionStatus transactionStatus = ackInfo.getTransactionStatus();
    if(ackedDirective != cfdp::FileDirectives::FINISH and
            ackedDirective != cfdp::FileDirectives::EOF_DIRECTIVE) {
        // TODO: better returncode
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    if(*size + 2 > maxSize) {
        return SerializeIF::BUFFER_TOO_SHORT;
    }
    **buffer = ackedDirective << 4 | directiveSubtypeCode;
    *buffer += 1;
    *size += 1;
    **buffer = ackedConditionCode << 4 | transactionStatus;
    *buffer += 1;
    *size += 1;
    return HasReturnvaluesIF::RETURN_OK;
}
