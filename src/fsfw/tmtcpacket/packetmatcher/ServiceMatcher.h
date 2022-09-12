#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_

#include "../../globalfunctions/matching/SerializeableMatcherIF.h"
#include "../../serialize/SerializeAdapter.h"
#include "../pus/tm/PusTmMinimal.h"

class ServiceMatcher : public SerializeableMatcherIF<PusTmIF*> {
 private:
  uint8_t service;

 public:
  explicit ServiceMatcher(uint8_t setService) : service(setService) {}
  explicit ServiceMatcher(PusTmIF* test) : service(test->getService()) {}
  bool match(PusTmIF* packet) override {
    if (packet->getService() == service) {
      return true;
    } else {
      return false;
    }
  }
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const override {
    return SerializeAdapter::serialize(&service, buffer, size, maxSize, streamEndianness);
  }
  [[nodiscard]] size_t getSerializedSize() const override {
    return SerializeAdapter::getSerializedSize(&service);
  }
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            Endianness streamEndianness) override {
    return SerializeAdapter::deSerialize(&service, buffer, size, streamEndianness);
  }
};

#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_ */
