#include <framework/tmtcservices/PusParser.h>

PusParser::PusParser(uint16_t maxExpectedPusPackets,
		bool storeSplitPackets): indexSizePairFIFO(maxExpectedPusPackets) {
}

ReturnValue_t PusParser::parsePusPackets(const uint8_t *frame,
		size_t frameSize) {
	return HasReturnvaluesIF::RETURN_OK;
}
