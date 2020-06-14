#include <framework/housekeeping/HousekeepingMessage.h>
#include <cstring>

HousekeepingMessage::HousekeepingMessage(MessageQueueMessageIF* message):
		CommandMessageBase(message) {
}

HousekeepingMessage::~HousekeepingMessage() {
}

void HousekeepingMessage::setHkReportMessage(sid_t sid,
		store_address_t storeId) {
	CommandMessageBase::setCommand(HK_REPORT);
	setSid(sid);
	setParameter(storeId.raw);
}

size_t HousekeepingMessage::getMinimumMessageSize() const {
	return HK_MESSAGE_SIZE;
}

size_t HousekeepingMessage::getMaximumMessageSize() const {
	return MessageQueueMessage::MAX_MESSAGE_SIZE;
}

void HousekeepingMessage::clear() {
	// clear IPC store where it is needed.
}

sid_t HousekeepingMessage::getSid() const {
	sid_t sid;
	std::memcpy(&sid.raw, CommandMessageBase::getData(), sizeof(sid.raw));
	return sid;
}

uint8_t* HousekeepingMessage::getData() {
	return internalMessage->getBuffer() + sizeof(sid_t);
}

void HousekeepingMessage::setParameter(uint32_t parameter) {
	memcpy(getData(), &parameter, sizeof(parameter));
}

void HousekeepingMessage::setSid(sid_t sid) {
	std::memcpy(CommandMessageBase::getData(), &sid.raw, sizeof(sid.raw));
}
