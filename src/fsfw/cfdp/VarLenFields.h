#ifndef FSFW_CFDP_PDU_VARLENFIELD_H_
#define FSFW_CFDP_PDU_VARLENFIELD_H_

#include <cstddef>
#include <cstdint>
#include <utility>

#include "fsfw/cfdp/definitions.h"
#include "fsfw/serialize/SerializeIF.h"
#include "fsfw/serviceinterface.h"
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

  bool operator==(const VarLenField &other) const;
  bool operator!=(const VarLenField &other) const;
  bool operator<(const VarLenField &other) const;

  ReturnValue_t setValue(cfdp::WidthInBytes, size_t value);

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

  ReturnValue_t deSerialize(cfdp::WidthInBytes width, const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness);

  [[nodiscard]] cfdp::WidthInBytes getWidth() const;
  [[nodiscard]] size_t getValue() const;

#if FSFW_CPP_OSTREAM_ENABLED == 1
  friend std::ostream &operator<<(std::ostream &os, const VarLenField &id) {
    os << "dec: " << id.getValue() << ", hex: " << std::hex << std::setw(id.getWidth())
       << std::setfill('0') << id.getValue() << std::dec << std::setfill('0');
    return os;
  }
#endif

 private:
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;

  cfdp::WidthInBytes width;
  LengthFieldLen value{};
};

template <typename T>
cfdp::VarLenField::VarLenField(UnsignedByteField<T> byteField)
    : width(static_cast<cfdp::WidthInBytes>(sizeof(T))) {
  static_assert((sizeof(T) % 2) == 0);
  setValue(width, byteField.getValue());
}

struct EntityId : public VarLenField {
 public:
  EntityId() : VarLenField() {}
  template <typename T>
  explicit EntityId(UnsignedByteField<T> byteField) : VarLenField(byteField) {}
  EntityId(cfdp::WidthInBytes width, size_t entityId) : VarLenField(width, entityId) {}
};

struct TransactionSeqNum : public VarLenField {
 public:
  TransactionSeqNum() : VarLenField() {}
  template <typename T>
  explicit TransactionSeqNum(UnsignedByteField<T> byteField) : VarLenField(byteField) {}
  TransactionSeqNum(cfdp::WidthInBytes width, size_t seqNum) : VarLenField(width, seqNum) {}
};

struct TransactionId {
  TransactionId() = default;
  TransactionId(EntityId entityId, TransactionSeqNum seqNum)
      : entityId(std::move(entityId)), seqNum(std::move(seqNum)) {}

  bool operator==(const TransactionId &other) const {
    return entityId == other.entityId and seqNum == other.seqNum;
  }

#if FSFW_CPP_OSTREAM_ENABLED == 1
  friend std::ostream &operator<<(std::ostream &os, const TransactionId &id) {
    os << "Source ID { " << id.entityId << " }, Sequence Number " << id.seqNum.getValue();
    return os;
  }
#endif
  EntityId entityId;
  TransactionSeqNum seqNum;
};

}  // namespace cfdp

#endif /* FSFW_CFDP_PDU_VARLENFIELD_H_ */
