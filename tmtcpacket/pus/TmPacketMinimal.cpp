#include "TmPacketMinimal.h"
#include <stddef.h>
#include <time.h>
#include "PacketTimestampInterpreterIF.h"

TmPacketMinimal::TmPacketMinimal(const uint8_t* set_data) : SpacePacketBase( set_data ) {
	this->tm_data = (TmPacketMinimalPointer*)set_data;
}

TmPacketMinimal::~TmPacketMinimal() {
}

uint8_t TmPacketMinimal::getService() {
	return tm_data->data_field.service_type;
}

uint8_t TmPacketMinimal::getSubService() {
	return tm_data->data_field.service_subtype;
}

uint8_t TmPacketMinimal::getPacketSubcounter() {
	return tm_data->data_field.subcounter;
}

ReturnValue_t TmPacketMinimal::getPacketTime(timeval* timestamp) {
	if (timestampInterpreter == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return timestampInterpreter->getPacketTime(this, timestamp);
}

ReturnValue_t TmPacketMinimal::getPacketTimeRaw(const uint8_t** timePtr, uint32_t* size) {
	if (timestampInterpreter == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return timestampInterpreter->getPacketTimeRaw(this, timePtr, size);
}

void TmPacketMinimal::setInterpretTimestampObject(PacketTimestampInterpreterIF* interpreter) {
	if (TmPacketMinimal::timestampInterpreter == NULL) {
		TmPacketMinimal::timestampInterpreter = interpreter;
	}
}

PacketTimestampInterpreterIF* TmPacketMinimal::timestampInterpreter = NULL;
