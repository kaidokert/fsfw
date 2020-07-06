#include <framework/tmtcservices/PusParser.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

PusParser::PusParser(uint16_t maxExpectedPusPackets,
		bool storeSplitPackets): indexSizePairFIFO(maxExpectedPusPackets) {
}

ReturnValue_t PusParser::parsePusPackets(const uint8_t *frame,
		size_t frameSize) {
	if(frame == nullptr) {
		sif::error << "PusParser::parsePusPackets: Frame pointers in invalid!"
				<< std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	// Size of a pus packet is the value in the packet length field plus 7.
	uint16_t packetSize = (frame[4] << 8 | frame[5]) + 7;
	if(packetSize > 0) {
		indexSizePairFIFO.insert(indexSizePair(0, packetSize));
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return NO_PACKET_FOUND;
	}

	if(frameSize > packetSize) {
		return readMultiplePackets(frameSize);

	}
	return HasReturnvaluesIF::RETURN_OK;
}

//ReturnValue_t PusParser::readMultiplePackets(size_t frameSize) {
//	size_t endOfBuffer = frameSize - 1;
//	size_t endIndex = firstPacketSize;
//	size_t startIndex = 0;
//	while (endIndex < endOfBuffer) {
//		ReturnValue_t result = readNextPacket(&startIndex, &endIndex);
//		if(result != RETURN_OK) {
//			return;
//		}
//	}
//}
