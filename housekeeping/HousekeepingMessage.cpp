#include "HousekeepingMessage.h"
#include <cstring>

HousekeepingMessage::~HousekeepingMessage() {}

void HousekeepingMessage::setHkReportMessage(CommandMessage* message, sid_t sid,
		store_address_t storeId) {
	message->setCommand(HK_REPORT);
	message->setMessageSize(HK_MESSAGE_SIZE);
	setSid(message, sid);
	message->setParameter3(storeId.raw);
}

void HousekeepingMessage::setHkDiagnosticsMessage(CommandMessage* message,
		sid_t sid, store_address_t storeId) {
	message->setCommand(DIAGNOSTICS_REPORT);
	message->setMessageSize(HK_MESSAGE_SIZE);
	setSid(message, sid);
	message->setParameter3(storeId.raw);
}

sid_t HousekeepingMessage::getHkReportMessage(const CommandMessage *message,
		store_address_t *storeIdToSet) {
	if(storeIdToSet != nullptr) {
	    *storeIdToSet = message->getParameter3();
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

