#include <framework/housekeeping/HousekeepingMessage.h>
#include <cstring>

HousekeepingMessage::HousekeepingMessage(MessageQueueMessageIF* message):
		CommandMessageBase(message) {
	if(message->getMaximumMessageSize() < HK_MESSAGE_SIZE) {
		sif::error << "CommandMessage::ComandMessage: Passed message buffer"
				" can not hold minimum "<< HK_MESSAGE_SIZE
				<< " bytes!" << std::endl;
		return;
	}
	message->setMessageSize(HK_MESSAGE_SIZE);
}

HousekeepingMessage::~HousekeepingMessage() {}

void HousekeepingMessage::setParameter(uint32_t parameter) {
	std::memcpy(getData(), &parameter, sizeof(parameter));
}

uint32_t HousekeepingMessage::getParameter() const {
	uint32_t parameter;
	std::memcpy(&parameter, getData(), sizeof(parameter));
	return parameter;
}

void HousekeepingMessage::setHkReportMessage(sid_t sid,
		store_address_t storeId) {
	CommandMessageBase::setCommand(HK_REPORT);
	setSid(sid);
	setParameter(storeId.raw);
}

void HousekeepingMessage::setHkDiagnosticsMessage(sid_t sid,
		store_address_t storeId) {
	CommandMessageBase::setCommand(DIAGNOSTICS_REPORT);
	setSid(sid);
	setParameter(storeId.raw);
}

size_t HousekeepingMessage::getMinimumMessageSize() const {
	return HK_MESSAGE_SIZE;
}

sid_t HousekeepingMessage::getSid() const {
	sid_t sid;
	std::memcpy(&sid.raw, CommandMessageBase::getData(), sizeof(sid.raw));
	return sid;
}


sid_t HousekeepingMessage::getHkReportMessage(
		store_address_t *storeIdToSet) const {
	if(storeIdToSet != nullptr) {
		*storeIdToSet = getParameter();
	}
	return getSid();
}

void HousekeepingMessage::setSid(sid_t sid) {
	std::memcpy(CommandMessageBase::getData(), &sid.raw, sizeof(sid.raw));
}


uint8_t* HousekeepingMessage::getData() {
	return CommandMessageBase::getData() + sizeof(sid_t);
}

const uint8_t* HousekeepingMessage::getData() const {
	return CommandMessageBase::getData() + sizeof(sid_t);
}

void HousekeepingMessage::clear() {
	// clear IPC store where it is needed.
}
