#ifndef FSFW_SRC_FSFW_CFDP_PDU_VARLENFIELD_H_
#define FSFW_SRC_FSFW_CFDP_PDU_VARLENFIELD_H_

#include <cstddef>
#include <cstdint>

#include "fsfw/cfdp/definitions.h"
#include "fsfw/serialize/SerializeIF.h"
#include "fsfw/util/UnsignedByteField.h"
namespace cfdp {

class VarLenField : public SerializeIF {
 public:
  union LengthFieldLen {
    uint8_t oneByte;
    uint16_t twoBytes;
    uint32_t fourBytes;
    uint64_t eightBytes;
  };

  VarLenField();
  template <typename T>
  explicit VarLenField(UnsignedByteField<T> byteField);
  VarLenField(cfdp::WidthInBytes width, size_t value);

  ReturnValue_t setValue(cfdp::WidthInBytes, size_t value);

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t deSerialize(cfdp::WidthInBytes width, const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness);

  [[nodiscard]] cfdp::WidthInBytes getWidth() const;
  [[nodiscard]] size_t getValue() const;

 private:
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;

  cfdp::WidthInBytes width;
  LengthFieldLen value;
};

}  // namespace cfdp

#endif /* FSFW_SRC_FSFW_CFDP_PDU_VARLENFIELD_H_ */
