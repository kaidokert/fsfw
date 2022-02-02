#ifndef FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_
#define FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_

#include "TlvIF.h"

namespace cfdp {

enum FaultHandlerCode {
    RESERVED = 0b0000,
    NOTICE_OF_CANCELLATION = 0b0001,
    NOTICE_OF_SUSPENSION = 0b0010,
    IGNORE_ERROR = 0b0011,
    ABANDON_TRANSACTION = 0b0100
};

}

class FaultHandlerOverrideTlv: public TlvIF {
public:

    FaultHandlerOverrideTlv();
    FaultHandlerOverrideTlv(cfdp::ConditionCode conditionCode, cfdp::FaultHandlerCode handlerCode);

    ReturnValue_t serialize(uint8_t **buffer, size_t *size,
            size_t maxSize, Endianness streamEndianness) const override;

    size_t getSerializedSize() const override;

    ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
            Endianness streamEndianness) override;
    uint8_t getLengthField() const override;
    cfdp::TlvTypes getType() const override;

private:
    cfdp::ConditionCode conditionCode = cfdp::ConditionCode::NO_CONDITION_FIELD;
    cfdp::FaultHandlerCode handlerCode = cfdp::FaultHandlerCode::RESERVED;
};

#endif /* FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_ */
