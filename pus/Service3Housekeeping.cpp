#include "Service3Housekeeping.h"
#include "servicepackets/Service3Packets.h"
#include "../datapoollocal/HasLocalDataPoolIF.h"


Service3Housekeeping::Service3Housekeeping(object_id_t objectId, uint16_t apid,
			uint8_t serviceId):
		CommandingServiceBase(objectId, apid, serviceId,
		NUM_OF_PARALLEL_COMMANDS, COMMAND_TIMEOUT_SECONDS) {}

Service3Housekeeping::~Service3Housekeeping() {}

ReturnValue_t Service3Housekeeping::isValidSubservice(uint8_t subservice) {
	switch(static_cast<Subservice>(subservice)) {
	case Subservice::ENABLE_PERIODIC_HK_REPORT_GENERATION:
	case Subservice::DISABLE_PERIODIC_HK_REPORT_GENERATION:
	case Subservice::ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
	case Subservice::DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
	case Subservice::REPORT_HK_REPORT_STRUCTURES:
	case Subservice::REPORT_DIAGNOSTICS_REPORT_STRUCTURES :
	case Subservice::GENERATE_ONE_PARAMETER_REPORT:
	case Subservice::GENERATE_ONE_DIAGNOSTICS_REPORT:
	case Subservice::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL:
	case Subservice::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL:
		return HasReturnvaluesIF::RETURN_OK;
	// Telemetry or invalid subservice.
	case Subservice::HK_DEFINITIONS_REPORT:
	case Subservice::DIAGNOSTICS_DEFINITION_REPORT:
	case Subservice::HK_REPORT:
	case Subservice::DIAGNOSTICS_REPORT:
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t Service3Housekeeping::getMessageQueueAndObject(uint8_t subservice,
		const uint8_t *tcData, size_t tcDataLen,
		MessageQueueId_t *id, object_id_t *objectId) {
	return HasReturnvaluesIF::RETURN_OK;
	ReturnValue_t result = checkAndAcquireTargetID(objectId,tcData,tcDataLen);
	if(result != RETURN_OK) {
		return result;
	}
	return checkInterfaceAndAcquireMessageQueue(id,objectId);
}

ReturnValue_t Service3Housekeeping::checkAndAcquireTargetID(
		object_id_t* objectIdToSet, const uint8_t* tcData, size_t tcDataLen) {
	if(SerializeAdapter::deSerialize(objectIdToSet, &tcData, &tcDataLen,
	        SerializeIF::Endianness::BIG) != HasReturnvaluesIF::RETURN_OK) {
		return CommandingServiceBase::INVALID_TC;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::checkInterfaceAndAcquireMessageQueue(
		MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
	// check OwnsLocalDataPoolIF property of target
	HasLocalDataPoolIF* possibleTarget =
			objectManager->get<HasLocalDataPoolIF>(*objectId);
	if(possibleTarget == nullptr){
		return CommandingServiceBase::INVALID_OBJECT;
	}
	*messageQueueToSet = possibleTarget->getCommandQueue();
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service3Housekeeping::prepareCommand(CommandMessage* message,
		uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
		uint32_t *state, object_id_t objectId) {
	switch(static_cast<Subservice>(subservice)) {
	case Subservice::ENABLE_PERIODIC_HK_REPORT_GENERATION:
		return prepareReportingTogglingCommand(message, true, false,
				tcData, tcDataLen);
	case Subservice::DISABLE_PERIODIC_HK_REPORT_GENERATION:
		return prepareReportingTogglingCommand(message, false, false,
				tcData, tcDataLen);
	case Subservice::ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
		return prepareReportingTogglingCommand(message, true, true,
				tcData, tcDataLen);
	case Subservice::DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
		return prepareReportingTogglingCommand(message, false, true,
				tcData, tcDataLen);
	case Subservice::REPORT_HK_REPORT_STRUCTURES:
		return prepareStructureReportingCommand(message, false, tcData,
				tcDataLen);
	case Subservice::REPORT_DIAGNOSTICS_REPORT_STRUCTURES:
		return prepareStructureReportingCommand(message, true, tcData,
				tcDataLen);
	case Subservice::GENERATE_ONE_PARAMETER_REPORT:
		return prepareOneShotReportCommand(message, false, tcData, tcDataLen);
	case Subservice::GENERATE_ONE_DIAGNOSTICS_REPORT:
		return prepareOneShotReportCommand(message, true, tcData, tcDataLen);
	case Subservice::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL:
		return prepareCollectionIntervalModificationCommand(message, false,
				tcData, tcDataLen);
	case Subservice::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL:
		return prepareCollectionIntervalModificationCommand(message, true,
				tcData, tcDataLen);
	case Subservice::HK_DEFINITIONS_REPORT:
	case Subservice::DIAGNOSTICS_DEFINITION_REPORT:
	case Subservice::HK_REPORT:
	case Subservice::DIAGNOSTICS_REPORT:
		// Those are telemetry packets.
		return CommandingServiceBase::INVALID_TC;
	default:
		// should never happen, subservice was already checked.
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::handleReply(const CommandMessage* reply,
		Command_t previousCommand, uint32_t *state,
		CommandMessage* optionalNextCommand, object_id_t objectId,
		bool *isStep) {
	switch(reply->getCommand()) {
	case(HousekeepingMessage::HK_REQUEST_SUCCESS): {
		return CommandingServiceBase::EXECUTION_COMPLETE;
	}
	case(HousekeepingMessage::HK_REQUEST_FAILURE): {
		failureParameter1 = objectId;
		// also provide failure reason (returnvalue)
		// will be most commonly invalid SID or the set already has the desired
		// reporting status.
		return CommandingServiceBase::EXECUTION_COMPLETE;
	}
	default:
		sif::error << "Service3Housekeeping::handleReply: Invalid reply!"
				<< std::endl;
		return CommandingServiceBase::INVALID_REPLY;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::prepareReportingTogglingCommand(
		CommandMessage *command, bool enableReporting, bool isDiagnostics,
		const uint8_t* tcData, size_t tcDataLen) {
	if(tcDataLen < sizeof(sid_t)) {
		// It is assumed the full SID is sent for now (even if that means
		// 4 bytes are redundant)
		return CommandingServiceBase::INVALID_TC;
	}

	sid_t targetSid;
	SerializeAdapter::deSerialize(&targetSid.objectId, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&targetSid.ownerSetId, &tcData, &tcDataLen,
				SerializeIF::Endianness::BIG);

	HousekeepingMessage::setToggleReportingCommand(command, targetSid,
			enableReporting, isDiagnostics);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::prepareStructureReportingCommand(
		CommandMessage *command, bool isDiagnostics, const uint8_t* tcData,
		size_t tcDataLen) {
	if(tcDataLen < sizeof(sid_t)) {
		// It is assumed the full SID is sent for now (even if that means
		// 4 bytes are redundant)
		return CommandingServiceBase::INVALID_TC;
	}

	sid_t targetSid;
	SerializeAdapter::deSerialize(&targetSid.objectId, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&targetSid.ownerSetId, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);

	HousekeepingMessage::setStructureReportingCommand(command, targetSid,
			isDiagnostics);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::prepareOneShotReportCommand(
		CommandMessage *command, bool isDiagnostics, const uint8_t *tcData,
		size_t tcDataLen) {
	if(tcDataLen < sizeof(sid_t)) {
		// It is assumed the full SID is sent for now (even if that means
		// 4 bytes are redundant)
		return CommandingServiceBase::INVALID_TC;
	}

	sid_t targetSid;
	SerializeAdapter::deSerialize(&targetSid.objectId, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&targetSid.ownerSetId, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);
	HousekeepingMessage::setOneShotReportCommand(command, targetSid,
			isDiagnostics);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service3Housekeeping::prepareCollectionIntervalModificationCommand(
		CommandMessage *command, bool isDiagnostics, const uint8_t *tcData,
		size_t tcDataLen) {
	if(tcDataLen < sizeof(sid_t) + sizeof(float)) {
		// It is assumed the full SID and the new collection interval as a float
		// is sent for now (even if that means 4 bytes are redundant)
		return CommandingServiceBase::INVALID_TC;
	}

	sid_t targetSid;
	float newCollectionInterval = 0;
	SerializeAdapter::deSerialize(&targetSid.objectId, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&targetSid.ownerSetId, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&newCollectionInterval, &tcData, &tcDataLen,
			SerializeIF::Endianness::BIG);
	HousekeepingMessage::setCollectionIntervalModificationCommand(command,
			targetSid, newCollectionInterval, isDiagnostics);
	return HasReturnvaluesIF::RETURN_OK;
}

void Service3Housekeeping::handleUnrequestedReply(
        CommandMessage* reply) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    switch(reply->getCommand()) {
    case(HousekeepingMessage::DIAGNOSTICS_REPORT): {
    	result = generateHkReport(reply,
    			static_cast<uint8_t>(Subservice::DIAGNOSTICS_REPORT));
    	break;
    }
    case(HousekeepingMessage::HK_REPORT): {
        result = generateHkReport(reply,
        		static_cast<uint8_t>(Subservice::HK_REPORT));
        break;
    }
    default: {
        sif::error << "Service3Housekeeping::handleUnrequestedReply: "
                << "Invalid reply!" << std::endl;
    }
    }

	if(result != HasReturnvaluesIF::RETURN_OK) {
		// Configuration error
		sif::debug << "Service3Housekeeping::handleUnrequestedReply:"
				<< "Could not generate reply!" << std::endl;
	}
}

MessageQueueId_t Service3Housekeeping::getHkQueue() const {
	return commandQueue->getId();
}

ReturnValue_t Service3Housekeeping::generateHkReport(
		const CommandMessage* hkMessage, uint8_t subserviceId) {
	store_address_t storeId;

	sid_t sid = HousekeepingMessage::getHkDataReply(hkMessage, &storeId);
	auto resultPair = IPCStore->getData(storeId);
	if(resultPair.first != HasReturnvaluesIF::RETURN_OK) {
		return resultPair.first;
	}

	HkPacket hkPacket(sid, resultPair.second.data(), resultPair.second.size());
	return sendTmPacket(static_cast<uint8_t>(subserviceId),
			hkPacket.hkData, hkPacket.hkSize, nullptr, 0);
}
