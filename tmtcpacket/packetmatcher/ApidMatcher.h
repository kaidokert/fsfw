#ifndef FRAMEWORK_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_
#define FRAMEWORK_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_

#include <framework/globalfunctions/matching/SerializeableMatcherIF.h>
#include <framework/serialize/SerializeAdapter.h>
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>

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
			const size_t max_size, bool bigEndian) const {
		return SerializeAdapter<uint16_t>::serialize(&apid, buffer,
				size, max_size, bigEndian);
	}
	size_t getSerializedSize() const {
		return SerializeAdapter<uint16_t>::getSerializedSize(&apid);
	}
	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
			bool bigEndian) {
		return SerializeAdapter<uint16_t>::deSerialize(&apid, buffer, size, bigEndian);
	}
};



#endif /* FRAMEWORK_TMTCPACKET_PACKETMATCHER_APIDMATCHER_H_ */
