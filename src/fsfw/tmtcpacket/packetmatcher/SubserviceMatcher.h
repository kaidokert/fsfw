#ifndef FSFW_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_
#define FSFW_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_

#include "../../globalfunctions/matching/SerializeableMatcherIF.h"
#include "../../serialize/SerializeAdapter.h"
#include "../pus/tm/PusTmMinimal.h"

class SubServiceMatcher : public SerializeableMatcherIF<PusTmIF*> {
 public:
  explicit SubServiceMatcher(uint8_t subService) : subService(subService) {}
  explicit SubServiceMatcher(PusTmIF* test) : subService(test->getSubService()) {}
  bool match(PusTmIF* packet) override {
    if (packet->getSubService() == subService) {
      return true;
    } else {
      return false;
    }
  }
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override {
    return SerializeAdapter::serialize(&subService, buffer, size, maxSize, streamEndianness);
  }
  [[nodiscard]] size_t getSerializedSize() const override {
    return SerializeAdapter::getSerializedSize(&subService);
  }
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    return SerializeAdapter::deSerialize(&subService, buffer, size, streamEndianness);
  }

 private:
  uint8_t subService{};
};

#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_ */
