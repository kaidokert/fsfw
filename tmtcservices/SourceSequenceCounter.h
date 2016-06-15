/**
 * @file	SourceSequenceCounter.h
 * @brief	This file defines the SourceSequenceCounter class.
 * @date	04.02.2013
 * @author	baetz
 */

#ifndef SOURCESEQUENCECOUNTER_H_
#define SOURCESEQUENCECOUNTER_H_
#include <framework/tmtcpacket/SpacePacketBase.h>

class SourceSequenceCounter {
private:
	uint16_t sequenceCount;
public:
	SourceSequenceCounter() : sequenceCount(0) {}
	void increment() {
		this->sequenceCount = (++this->sequenceCount) % (SpacePacketBase::LIMIT_SEQUENCE_COUNT);
	}
	void decrement() {
		this->sequenceCount = (--this->sequenceCount) % (SpacePacketBase::LIMIT_SEQUENCE_COUNT);
	}
	uint16_t get() { return this->sequenceCount; }
	void reset(uint16_t toValue = 0) {
		sequenceCount = toValue % (SpacePacketBase::LIMIT_SEQUENCE_COUNT);
	}
};


#endif /* SOURCESEQUENCECOUNTER_H_ */
