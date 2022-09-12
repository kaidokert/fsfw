#ifndef FSFW_TMTCSERVICES_SOURCESEQUENCECOUNTER_H_
#define FSFW_TMTCSERVICES_SOURCESEQUENCECOUNTER_H_

#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"

class SourceSequenceCounter {
 private:
  uint16_t sequenceCount;

 public:
  SourceSequenceCounter() : sequenceCount(0) {}
  void increment() {
    sequenceCount = (sequenceCount + 1) % (SpacePacketBase::LIMIT_SEQUENCE_COUNT);
  }
  void decrement() {
    sequenceCount = (sequenceCount - 1) % (SpacePacketBase::LIMIT_SEQUENCE_COUNT);
  }
  uint16_t get() { return this->sequenceCount; }
  void reset(uint16_t toValue = 0) {
    sequenceCount = toValue % (SpacePacketBase::LIMIT_SEQUENCE_COUNT);
  }
};

#endif /* FSFW_TMTCSERVICES_SOURCESEQUENCECOUNTER_H_ */
