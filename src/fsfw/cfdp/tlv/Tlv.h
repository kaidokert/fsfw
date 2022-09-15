#ifndef FSFW_SRC_FSFW_CFDP_TLV_H_
#define FSFW_SRC_FSFW_CFDP_TLV_H_

#include "TlvIF.h"
#include "fsfw/serialize/SerialBufferAdapter.h"

namespace cfdp {

/**
 * @brief Type-Length-Value field implementation
 * @details
 * Thin abstraction layer around a serial buffer adapter
 */
class Tlv : public TlvIF {
 public:
  Tlv(TlvType type, const uint8_t *value, size_t size);
  Tlv();

  /**
   * Serialize a TLV into a given buffer
   * @param buffer
   * @param size
   * @param maxSize
   * @param streamEndianness
   * @return
   *  - returnvalue::OK on success
   *  - INVALID_TLV_TYPE
   *  - SerializeIF returncode on wrong serialization parameters
   */
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

  /**
   * @brief Deserialize a LV field from a raw buffer. Zero-copy implementation
   * @param buffer Raw buffer including the size field
   * @param size
   * @param streamEndianness
   *  - returnvalue::OK on success
   *  - INVALID_TLV_TYPE
   *  - SerializeIF returncode on wrong deserialization parameters
   */
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;

  void setValue(uint8_t *value, size_t len);

  [[nodiscard]] const uint8_t *getValue() const;
  void setType(TlvType type);
  [[nodiscard]] TlvType getType() const override;
  [[nodiscard]] uint8_t getLengthField() const override;

 private:
  bool checkType(uint8_t rawType);

  bool zeroLen = true;
  TlvType type = TlvType::INVALID_TLV;
  SerialBufferAdapter<uint8_t> value;
};

}  // namespace cfdp

#endif /* FSFW_SRC_FSFW_CFDP_TLV_H_ */
