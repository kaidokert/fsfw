#ifndef FSFW_TIMEMANAGER_TIMESTAMPER_H_
#define FSFW_TIMEMANAGER_TIMESTAMPER_H_

#include "CCSDSTime.h"
#include "TimeReaderIF.h"
#include "TimeStamperIF.h"
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
class CdsShortTimeStamper : public TimeStamperIF, public TimeReaderIF, public SystemObject {
 public:
  static constexpr size_t TIMESTAMP_LEN = 7;
  /**
   * @brief   Default constructor which also registers the time stamper as a
   *          system object so it can be found with the #objectManager.
   * @param objectId
   */
  explicit CdsShortTimeStamper(object_id_t objectId);

  /**
   * Adds a CCSDS CDC short 8 byte timestamp to the given buffer.
   * This function can be overriden to use a custom timestamp.
   * @param buffer
   * @param maxSize
   * @return
   */
  ReturnValue_t addTimeStamp(uint8_t *buffer, uint8_t maxSize) override;
  ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                          Endianness streamEndianness) const override;
  [[nodiscard]] size_t getSerializedSize() const override;
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override;
  ReturnValue_t readTimeStamp(const uint8_t *buffer, size_t maxSize) override;
  timeval &getTime() override;
  [[nodiscard]] size_t getTimestampSize() const override;

 private:
  timeval readTime{};
};

#endif /* FSFW_TIMEMANAGER_TIMESTAMPER_H_ */
