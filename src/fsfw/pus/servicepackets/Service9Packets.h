#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE9PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE9PACKETS_H_

#include "fsfw/serialize/SerialLinkedListAdapter.h"

/**
 * @brief Subservice 128
 * @details
 * It only contains the time encoded as ASCII, CRC, CUC or CDS
 * @ingroup spacepackets
 */
class TimePacket : SerialLinkedListAdapter<SerializeIF> {  //!< [EXPORT] : [SUBSERVICE] 128
 public:
  TimePacket(const TimePacket& command) = delete;
  TimePacket(const uint8_t* timeBuffer_, uint32_t timeSize_) {
    timeBuffer = timeBuffer_;
    timeSize = timeSize_;
  }
  const uint8_t* getTime() { return timeBuffer; }

  [[nodiscard]] uint32_t getTimeSize() const { return timeSize; }

 private:
  const uint8_t* timeBuffer;
  uint32_t timeSize;  //!< [EXPORT] : [IGNORE]
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE9PACKETS_H_ */
