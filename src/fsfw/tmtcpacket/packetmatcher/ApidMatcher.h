#ifndef FSFW_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_
#define FSFW_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_

#include "../../globalfunctions/matching/SerializeableMatcherIF.h"
#include "../../serialize/SerializeAdapter.h"
#include "../../tmtcpacket/pus/tm/TmPacketMinimal.h"

class ApidMatcher : public SerializeableMatcherIF<TmPacketMinimal*> {
 private:
  uint16_t apid;

 public:
  explicit ApidMatcher(uint16_t setApid) : apid(setApid) {}
  explicit ApidMatcher(TmPacketMinimal* test) : apid(test->getApid()) {}
  bool match(TmPacketMinimal* packet) override {
    if (packet->getApid() == apid) {
      return true;
    } else {
      return false;
    }
  }
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override {
    return SerializeAdapter::serialize(&apid, buffer, size, maxSize, streamEndianness);
  }
  [[nodiscard]] size_t getSerializedSize() const override {
    return SerializeAdapter::getSerializedSize(&apid);
  }
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    return SerializeAdapter::deSerialize(&apid, buffer, size, streamEndianness);
  }
};

#endif /* FSFW_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_ */
