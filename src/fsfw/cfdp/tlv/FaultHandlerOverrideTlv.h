#ifndef FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_
#define FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_

#include "TlvIF.h"

class FaultHandlerOverrideTlv : public TlvIF {
 public:
  FaultHandlerOverrideTlv();
  FaultHandlerOverrideTlv(cfdp::ConditionCode conditionCode, cfdp::FaultHandlerCode handlerCode);

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;
  [[nodiscard]] uint8_t getLengthField() const override;
  [[nodiscard]] cfdp::TlvType getType() const override;

 private:
  cfdp::ConditionCode conditionCode = cfdp::ConditionCode::NO_CONDITION_FIELD;
  cfdp::FaultHandlerCode handlerCode = cfdp::FaultHandlerCode::RESERVED;
};

#endif /* FSFW_SRC_FSFW_CFDP_TLV_FAULTHANDLEROVERRIDETLV_H_ */
