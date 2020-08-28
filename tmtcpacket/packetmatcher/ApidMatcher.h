#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_

#include "../../globalfunctions/matching/SerializeableMatcherIF.h"
#include "../../serialize/SerializeAdapter.h"
#include "../../tmtcpacket/pus/TmPacketMinimal.h"

class ApidMatcher: public SerializeableMatcherIF<TmPacketMinimal*> {
private:
	uint16_t apid;
public:
	ApidMatcher(uint16_t setApid) :
			apid(setApid) {
	}
	ApidMatcher(TmPacketMinimal* test) :
			apid(test->getAPID()) {
	}
	bool match(TmPacketMinimal* packet) {
		if (packet->getAPID() == apid) {
			return true;
		} else {
			return false;
		}
	}
	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const {
		return SerializeAdapter::serialize(&apid, buffer, size, maxSize, streamEndianness);
	}
	size_t getSerializedSize() const {
		return SerializeAdapter::getSerializedSize(&apid);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) {
		return SerializeAdapter::deSerialize(&apid, buffer, size, streamEndianness);
	}
};



#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_ */
