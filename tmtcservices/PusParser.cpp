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

	if(indexSizePairFIFO.full()) {
		sif::error << "PusParser::parsePusPackets: FIFO is full!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}


	size_t lengthField = frame[4] << 8 | frame[5];

	if(lengthField == 0) {
		return NO_PACKET_FOUND;
	}

	size_t packetSize = lengthField + 7;
	if(lengthField > 0) {
		// Size of a pus packet is the value in the packet length field plus 7.
		if(packetSize > frameSize)
		{
			if(storeSplitPackets)
			{
				indexSizePairFIFO.insert(indexSizePair(0, frameSize));
			}
			else
			{
				sif::debug << "TcSerialPollingTask::readNextPacket: Next packet "
						"larger than remaining frame," << std::endl;
				sif::debug << "Throwing away packet. Detected packet size: "
						<< packetSize << std::endl;
			}
			return SPLIT_PACKET;
		}
		else
		{
			indexSizePairFIFO.insert(indexSizePair(0, packetSize));
			if(packetSize == frameSize) {
				return HasReturnvaluesIF::RETURN_OK;
			}
		}
	}

	// packet size is smaller than frame size, parse for more packets.
	return readMultiplePackets(frame, frameSize, packetSize);
}

ReturnValue_t PusParser::readMultiplePackets(const uint8_t *frame,
		size_t frameSize, size_t startIndex) {
	while (startIndex < frameSize) {
		ReturnValue_t result = readNextPacket(frame, frameSize, startIndex);
		if(result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

fsfw::FIFO<PusParser::indexSizePair>* PusParser::fifo(){
	return &indexSizePairFIFO;
}

PusParser::indexSizePair PusParser::getNextFifoPair() {
	indexSizePair nextIndexSizePair;
	indexSizePairFIFO.retrieve(&nextIndexSizePair);
	return nextIndexSizePair;
}

ReturnValue_t PusParser::readNextPacket(const uint8_t *frame,
		size_t frameSize, size_t& currentIndex) {
	// sif::debug << startIndex << std::endl;
	uint16_t lengthField = frame[currentIndex + 4] << 8 |
			frame[currentIndex + 5];
	if(lengthField == 0) {
		// It is assumed that no packet follows.
		currentIndex = frameSize;
		return HasReturnvaluesIF::RETURN_OK;
	}
	size_t nextPacketSize = lengthField + 7;
	size_t remainingSize = frameSize - currentIndex;
	if(nextPacketSize > remainingSize)
	{
		if(storeSplitPackets)
		{
			indexSizePairFIFO.insert(indexSizePair(currentIndex, remainingSize));
		}
		else
		{
			sif::debug << "TcSerialPollingTask::readNextPacket: Next packet "
					"larger than remaining frame," << std::endl;
			sif::debug << "Throwing away packet. Detected packet size: "
					<< nextPacketSize << std::endl;
		}
		return SPLIT_PACKET;
	}

	ReturnValue_t result = indexSizePairFIFO.insert(indexSizePair(currentIndex,
			nextPacketSize));
	if (result != HasReturnvaluesIF::RETURN_OK) {
		// FIFO full.
		sif::debug << "PusParser: Issue inserting into start index size "
		        "FIFO, it is full!" << std::endl;
	}
	currentIndex += nextPacketSize;

	return result;
}
