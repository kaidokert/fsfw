#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_

#include "../../globalfunctions/matching/SerializeableMatcherIF.h"
#include "../../serialize/SerializeAdapter.h"
#include "../../tmtcpacket/pus/TmPacketMinimal.h"

class SubServiceMatcher: public SerializeableMatcherIF<TmPacketMinimal*> {
public:
	SubServiceMatcher(uint8_t subService) :
			subService(subService) {
	}
	SubServiceMatcher(TmPacketMinimal* test) :
			subService(test->getSubService()) {
	}
	bool match(TmPacketMinimal* packet) {
		if (packet->getSubService() == subService) {
			return true;
		} else {
			return false;
		}
	}
	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const {
		return SerializeAdapter::serialize(&subService, buffer, size, maxSize, streamEndianness);
	}
	size_t getSerializedSize() const {
		return SerializeAdapter::getSerializedSize(&subService);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) {
		return SerializeAdapter::deSerialize(&subService, buffer, size, streamEndianness);
	}
private:
	uint8_t subService;
};



#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_ */
