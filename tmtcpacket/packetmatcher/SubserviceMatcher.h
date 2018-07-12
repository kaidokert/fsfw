#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_

#include <framework/globalfunctions/matching/SerializeableMatcherIF.h>
#include <framework/serialize/SerializeAdapter.h>
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>

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
	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		return SerializeAdapter<uint8_t>::serialize(&subService, buffer, size, max_size, bigEndian);
	}
	uint32_t getSerializedSize() const {
		return SerializeAdapter<uint8_t>::getSerializedSize(&subService);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return SerializeAdapter<uint8_t>::deSerialize(&subService, buffer, size, bigEndian);
	}
private:
	uint8_t subService;
};



#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_SUBSERVICEMATCHER_H_ */
