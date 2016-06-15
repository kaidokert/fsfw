#include <framework/monitoring/MonitoringMessage.h>
#include <framework/objectmanager/ObjectManagerIF.h>

MonitoringMessage::~MonitoringMessage() {
}

void MonitoringMessage::setAddLimitCommand(CommandMessage* message,
		store_address_t storeId) {
	setTypicalMessage(message, ADD_MONITOR, storeId);
}

void MonitoringMessage::setLimitViolationReport(CommandMessage* message,
		store_address_t storeId) {
	setTypicalMessage(message, LIMIT_VIOLATION_REPORT, storeId);
}

void MonitoringMessage::setTypicalMessage(CommandMessage* message,
		Command_t type, store_address_t storeId) {
	message->setCommand(type);
	message->setParameter2(storeId.raw);
}

store_address_t MonitoringMessage::getStoreId(const CommandMessage* message) {
	store_address_t temp;
	temp.raw = message->getParameter2();
	return temp;
}

void MonitoringMessage::clear(CommandMessage* message) {
	message->setCommand(CommandMessage::CMD_NONE);
	switch (message->getCommand()) {
	case MonitoringMessage::ADD_MONITOR:
	case MonitoringMessage::UPDATE_PARAMETER_MONITOR:
	case MonitoringMessage::LIMIT_VIOLATION_REPORT: {
		StorageManagerIF *ipcStore = objectManager->get<StorageManagerIF>(
				objects::IPC_STORE);
		if (ipcStore != NULL) {
			ipcStore->deleteData(getStoreId(message));
		}
		break;
	}
	default:
		break;
	}
}

void MonitoringMessage::setChangeReportingStrategy(CommandMessage* message,
		uint8_t strategy, store_address_t storeId) {
	message->setCommand(CHANGE_REPORTING_STRATEGY);
	message->setParameter(strategy);
	message->setParameter2(storeId.raw);
}

uint8_t MonitoringMessage::getReportingStategy(CommandMessage* message) {
	return (message->getParameter() & 0xFF);
}
