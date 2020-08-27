#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_

#include "../../globalfunctions/matching/SerializeableMatcherIF.h"
#include "../../serialize/SerializeAdapter.h"
#include "../../tmtcpacket/pus/TmPacketMinimal.h"

class ServiceMatcher: public SerializeableMatcherIF<TmPacketMinimal*> {
private:
	uint8_t service;
public:
	ServiceMatcher(uint8_t setService) :
			service(setService) {
	}
	ServiceMatcher(TmPacketMinimal* test) :
			service(test->getService()) {
	}
	bool match(TmPacketMinimal* packet) {
		if (packet->getService() == service) {
			return true;
		} else {
			return false;
		}
	}
	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const {
		return SerializeAdapter::serialize(&service, buffer, size, maxSize, streamEndianness);
	}
	size_t getSerializedSize() const {
		return SerializeAdapter::getSerializedSize(&service);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) {
		return SerializeAdapter::deSerialize(&service, buffer, size, streamEndianness);
	}
};


#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_ */
