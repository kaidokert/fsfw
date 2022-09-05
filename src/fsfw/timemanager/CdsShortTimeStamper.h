#ifndef FSFW_TIMEMANAGER_TIMESTAMPER_H_
#define FSFW_TIMEMANAGER_TIMESTAMPER_H_

#include "CCSDSTime.h"
#include "TimeReaderIF.h"
#include "TimeWriterIF.h"
#include "fsfw/objectmanager/SystemObject.h"

/**
 * @brief   Time stamper which can be used to add any timestamp to a
 *          given buffer.
 * @details
 * This time stamper uses the CCSDS CDC short timestamp as a fault timestamp.
 * This timestamp has a size of 8 bytes. A custom timestamp can be used by
 * overriding the #addTimeStamp function.
 * @ingroup utility
 */
class CdsShortTimeStamper : public TimeWriterIF, public TimeReaderIF, public SystemObject {
 public:
  static constexpr size_t TIMESTAMP_LEN = 7;
  /**
   * @brief   Default constructor which also registers the time stamper as a
   *          system object so it can be found with the #objectManager.
   * @param objectId
   */
  explicit CdsShortTimeStamper(object_id_t objectId);

  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;
  [[nodiscard]] size_t getSerializedSize() const override;
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;
  timeval &getTime() override;
  [[nodiscard]] size_t getTimestampSize() const override;

 private:
  timeval readTime{};
};

#endif /* FSFW_TIMEMANAGER_TIMESTAMPER_H_ */
