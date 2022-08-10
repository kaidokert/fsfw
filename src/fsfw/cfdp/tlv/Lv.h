#ifndef FSFW_CFDP_LV_H_
#define FSFW_CFDP_LV_H_

#include <vector>

#include "fsfw/serialize/SerialBufferAdapter.h"

namespace cfdp {

/**
 * @brief Length-Value field implementation
 * @details
 * Thin abstraction layer around a serial buffer adapter
 */
class Lv : public SerializeIF {
 public:
  explicit Lv(const std::vector<uint8_t>& data);
  Lv(const uint8_t* value, size_t size);
  Lv();

  Lv(const Lv&);
  Lv& operator=(const Lv&);

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override;

  [[nodiscard]] size_t getSerializedSize() const override;

  /**
   * @brief Deserialize a LV field from a raw buffer
   * @param buffer Raw buffer including the size field
   * @param size
   * @param streamEndianness
   * @return
   */
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override;

  /**
   * Get value field and its size.
   * @param size Optionally retrieve size. Size will be the size of the actual value field
   * without the length field of the LV
   * @return
   */
  const uint8_t* getValue(size_t* size) const;

 private:
  bool zeroLen = true;
  SerialBufferAdapter<uint8_t> value;
};

}  // namespace cfdp

#endif /* FSFW_CFDP_LV_H_ */
