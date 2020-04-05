#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_

#include <framework/globalfunctions/matching/SerializeableMatcherIF.h>
#include <framework/serialize/SerializeAdapter.h>
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>

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
			const size_t max_size, bool bigEndian) const {
		return SerializeAdapter<uint8_t>::serialize(&service, buffer,
				size, max_size, bigEndian);
	}
	uint32_t getSerializedSize() const {
		return SerializeAdapter<uint8_t>::getSerializedSize(&service);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return SerializeAdapter<uint8_t>::deSerialize(&service, buffer, size, bigEndian);
	}
};


#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_SERVICEMATCHER_H_ */
