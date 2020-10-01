#include <fsfw/objectmanager/ObjectManagerIF.h>
#include "HousekeepingMessage.h"
#include <cstring>

HousekeepingMessage::~HousekeepingMessage() {}

void HousekeepingMessage::setHkReportReply(CommandMessage* message, sid_t sid,
		store_address_t storeId) {
	message->setCommand(HK_REPORT);
	message->setMessageSize(HK_MESSAGE_SIZE);
	setSid(message, sid);
	message->setParameter3(storeId.raw);
}

void HousekeepingMessage::setHkDiagnosticsReply(CommandMessage* message,
		sid_t sid, store_address_t storeId) {
	message->setCommand(DIAGNOSTICS_REPORT);
	message->setMessageSize(HK_MESSAGE_SIZE);
	setSid(message, sid);
	message->setParameter3(storeId.raw);
}

sid_t HousekeepingMessage::getHkDataReply(const CommandMessage *message,
		store_address_t *storeIdToSet) {
	if(storeIdToSet != nullptr) {
	    *storeIdToSet = message->getParameter3();
	}
	return getSid(message);
}

void HousekeepingMessage::setToggleReportingCommand(CommandMessage *message,
		sid_t sid, bool enableReporting, bool isDiagnostics) {
	if(isDiagnostics) {
		if(enableReporting) {
			message->setCommand(ENABLE_PERIODIC_DIAGNOSTICS_GENERATION);
		}
		else {
			message->setCommand(DISABLE_PERIODIC_DIAGNOSTICS_GENERATION);
		}
	}
	else {
		if(enableReporting) {
			message->setCommand(ENABLE_PERIODIC_HK_REPORT_GENERATION);
		}
		else {
			message->setCommand(DISABLE_PERIODIC_HK_REPORT_GENERATION);
		}
	}

	setSid(message, sid);
}

void HousekeepingMessage::setStructureReportingCommand(CommandMessage *command,
		sid_t sid, bool isDiagnostics) {
	if(isDiagnostics) {
		command->setCommand(REPORT_DIAGNOSTICS_REPORT_STRUCTURES);
	}
	else {
		command->setCommand(REPORT_HK_REPORT_STRUCTURES);
	}

	setSid(command, sid);
}

void HousekeepingMessage::setOneShotReportCommand(CommandMessage *command,
		sid_t sid, bool isDiagnostics) {
	if(isDiagnostics) {
		command->setCommand(GENERATE_ONE_DIAGNOSTICS_REPORT);
	}
	else {
		command->setCommand(GENERATE_ONE_PARAMETER_REPORT);
	}

	setSid(command, sid);
}

void HousekeepingMessage::setCollectionIntervalModificationCommand(
		CommandMessage *command, sid_t sid, float collectionInterval,
		bool isDiagnostics) {
	if(isDiagnostics) {
		command->setCommand(MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL);
	}
	else {
		command->setCommand(MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL);
	}
	command->setParameter3(collectionInterval);

	setSid(command, sid);
}

sid_t HousekeepingMessage::getCollectionIntervalModificationCommand(
		const CommandMessage* command, float* newCollectionInterval) {
	if(newCollectionInterval != nullptr) {
		*newCollectionInterval = command->getParameter3();
	}

	return getSid(command);
}

void HousekeepingMessage::setHkRequestSuccessReply(CommandMessage *reply,
		sid_t sid) {
	setSid(reply, sid);
	reply->setCommand(HK_REQUEST_SUCCESS);
}

void HousekeepingMessage::setHkRequestFailureReply(CommandMessage *reply,
		sid_t sid, ReturnValue_t error) {
	setSid(reply, sid);
	reply->setCommand(HK_REQUEST_FAILURE);
	reply->setParameter3(error);
}

sid_t HousekeepingMessage::getHkRequestFailureReply(const CommandMessage *reply,
		ReturnValue_t *error) {
	if(error != nullptr) {
		*error = reply->getParameter3();
	}
	return getSid(reply);
}

sid_t HousekeepingMessage::getSid(const CommandMessage* message) {
	sid_t sid;
	std::memcpy(&sid.raw, message->getData(), sizeof(sid.raw));
	return sid;
}

void HousekeepingMessage::setSid(CommandMessage *message, sid_t sid) {
	std::memcpy(message->getData(), &sid.raw, sizeof(sid.raw));
}

void HousekeepingMessage::setHkStuctureReportReply(CommandMessage *reply,
        sid_t sid, store_address_t storeId) {
    reply->setCommand(HK_DEFINITIONS_REPORT);
    setSid(reply, sid);
    reply->setParameter3(storeId.raw);
}

void HousekeepingMessage::setDiagnosticsStuctureReportReply(
        CommandMessage *reply, sid_t sid, store_address_t storeId) {
    reply->setCommand(DIAGNOSTICS_DEFINITION_REPORT);
    setSid(reply, sid);
    reply->setParameter3(storeId.raw);
}

void HousekeepingMessage::clear(CommandMessage* message) {
    switch(message->getCommand()) {
    case(HK_REPORT):
    case(DIAGNOSTICS_REPORT):
    case(HK_DEFINITIONS_REPORT):
    case(DIAGNOSTICS_DEFINITION_REPORT):
    case(UPDATE_SNAPSHOT): {
        store_address_t storeId;
        getHkDataReply(message, &storeId);
        StorageManagerIF *ipcStore = objectManager->get<StorageManagerIF>(
                objects::IPC_STORE);
        if (ipcStore != nullptr) {
            ipcStore->deleteData(storeId);
        }
    }
    }
    message->setCommand(CommandMessage::CMD_NONE);
}
