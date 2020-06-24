#include <framework/housekeeping/HousekeepingMessage.h>
#include <cstring>

HousekeepingMessage::~HousekeepingMessage() {}

void HousekeepingMessage::setHkReportMessage(CommandMessage* message, sid_t sid,
		store_address_t storeId) {
	message->setCommand(HK_REPORT);
	message->setMessageSize(HK_MESSAGE_SIZE);
	setSid(message, sid);
	setParameter(message, storeId.raw);
}

void HousekeepingMessage::setHkDiagnosticsMessage(CommandMessage* message,
		sid_t sid, store_address_t storeId) {
	message->setCommand(DIAGNOSTICS_REPORT);
	message->setMessageSize(HK_MESSAGE_SIZE);
	setSid(message, sid);
	setParameter(message, storeId.raw);
}

sid_t HousekeepingMessage::getHkReportMessage(const CommandMessage *message,
		store_address_t *storeIdToSet) {
	if(storeIdToSet != nullptr) {
		*storeIdToSet = getParameter(message);
	}
	return getSid(message);
}

sid_t HousekeepingMessage::getSid(const CommandMessage* message) {
	sid_t sid;
	std::memcpy(&sid.raw, message->getData(), sizeof(sid.raw));
	return sid;
}

void HousekeepingMessage::setSid(CommandMessage *message, sid_t sid) {
	std::memcpy(message->getData(), &sid.raw, sizeof(sid.raw));
}


void HousekeepingMessage::setParameter(CommandMessage *message,
		uint32_t parameter) {
	message->setParameter3(parameter);
}

uint32_t HousekeepingMessage::getParameter(const CommandMessage *message) {
	return message->getParameter3();
}
