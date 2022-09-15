#ifndef FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_
#define FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_

#include "TlvIF.h"

class FaultHandlerOverrideTlv : public TlvIF {
 public:
  FaultHandlerOverrideTlv();
  FaultHandlerOverrideTlv(cfdp::ConditionCodes conditionCode, cfdp::FaultHandlerCodes handlerCode);

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;
  [[nodiscard]] uint8_t getLengthField() const override;
  [[nodiscard]] cfdp::TlvTypes getType() const override;

 private:
  cfdp::ConditionCodes conditionCode = cfdp::ConditionCodes::NO_CONDITION_FIELD;
  cfdp::FaultHandlerCodes handlerCode = cfdp::FaultHandlerCodes::RESERVED;
};

#endif /* FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_ */
