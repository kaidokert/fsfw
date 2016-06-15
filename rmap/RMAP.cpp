#include <framework/devicehandlers/DeviceCommunicationIF.h>
#include <framework/rmap/rmapStructs.h>
#include <framework/rmap/RMAP.h>
#include <framework/rmap/RMAPChannelIF.h>
#include <stddef.h>

ReturnValue_t RMAP::reset(RMAPCookie* cookie) {
	return cookie->channel->reset();
}

//TODO Check for channel == NULL
//Done. BB
RMAP::RMAP(){

}

ReturnValue_t RMAP::sendWriteCommand(RMAPCookie *cookie, uint8_t* buffer,
		uint32_t length) {
	uint8_t instruction;

	if ((buffer == NULL) && (length != 0)) {
		return DeviceCommunicationIF::NULLPOINTER;
	}
	if (cookie->getChannel() == NULL) {
		return COMMAND_NO_CHANNEL;
	}
	instruction = RMAP_COMMAND_WRITE | cookie->command_mask;
	return cookie->getChannel()->sendCommand(cookie, instruction, buffer,
			length);

}

ReturnValue_t RMAP::getWriteReply(RMAPCookie *cookie) {
	if (cookie->getChannel() == NULL) {
		return COMMAND_NO_CHANNEL;
	}
	if (cookie->header.instruction & (1 << RMAP_COMMAND_BIT_WRITE)) {
		return cookie->getChannel()->getReply(cookie, NULL, NULL);
	} else {
		return REPLY_MISSMATCH;
	}
}

ReturnValue_t RMAP::writeBlocking(RMAPCookie *cookie, uint8_t* buffer,
		uint32_t length, uint32_t timeout_us) {
	if (cookie->getChannel() == NULL) {
		return COMMAND_NO_CHANNEL;
	}
	return cookie->getChannel()->sendCommandBlocking(cookie, buffer, length,
			NULL, NULL, timeout_us);

}
ReturnValue_t RMAP::sendReadCommand(RMAPCookie *cookie, uint32_t expLength) {
	uint8_t command;
	if (cookie->getChannel() == NULL) {
		return COMMAND_NO_CHANNEL;
	}
	command = RMAP_COMMAND_READ
			| (cookie->command_mask & ~(1 << RMAP_COMMAND_BIT_VERIFY));

	return cookie->getChannel()->sendCommand(cookie, command, NULL, expLength);

}

ReturnValue_t RMAP::getReadReply(RMAPCookie *cookie, uint8_t **buffer,
		uint32_t *size) {
	if (cookie->getChannel() == NULL) {
		return COMMAND_NO_CHANNEL;
	}
	if (buffer == NULL || size == NULL) {
		return DeviceCommunicationIF::NULLPOINTER;
	}
	if (cookie->header.instruction & (1 << RMAP_COMMAND_BIT_WRITE)) {
		return REPLY_MISSMATCH;
	} else {
		return cookie->getChannel()->getReply(cookie, buffer, size);
	}

}

ReturnValue_t RMAP::readBlocking(RMAPCookie *cookie, uint32_t expLength,
		uint8_t** buffer, uint32_t *size, uint32_t timeout_us) {
	if (cookie->getChannel() == NULL) {
		return COMMAND_NO_CHANNEL;
	}
	if (buffer == NULL || size == NULL) {
		return DeviceCommunicationIF::NULLPOINTER;
	}
	return cookie->getChannel()->sendCommandBlocking(cookie, NULL, expLength,
			buffer, size, timeout_us);
}
