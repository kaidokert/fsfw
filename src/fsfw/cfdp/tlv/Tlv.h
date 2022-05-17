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
  Tlv(TlvTypes type, const uint8_t *value, size_t size);
  Tlv();

  /**
   * Serialize a TLV into a given buffer
   * @param buffer
   * @param size
   * @param maxSize
   * @param streamEndianness
   * @return
   *  - RETURN_OK on success
   *  - INVALID_TLV_TYPE
   *  - SerializeIF returncode on wrong serialization parameters
   */
  virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                  Endianness streamEndianness) const override;

  virtual size_t getSerializedSize() const override;

  /**
   * @brief Deserialize a LV field from a raw buffer. Zero-copy implementation
   * @param buffer Raw buffer including the size field
   * @param size
   * @param streamEndianness
   *  - RETURN_OK on success
   *  - INVALID_TLV_TYPE
   *  - SerializeIF returncode on wrong deserialization parameters
   */
  virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                                    Endianness streamEndianness) override;

  void setValue(uint8_t *value, size_t len);

  const uint8_t *getValue() const;
  void setType(TlvTypes type);
  TlvTypes getType() const override;
  uint8_t getLengthField() const override;

 private:
  bool checkType(uint8_t rawType);

  bool zeroLen = true;
  TlvTypes type = TlvTypes::INVALID_TLV;
  SerialBufferAdapter<uint8_t> value;
};

}  // namespace cfdp

#endif /* FSFW_SRC_FSFW_CFDP_TLV_H_ */
