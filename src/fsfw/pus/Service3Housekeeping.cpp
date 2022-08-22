#include "fsfw/pus/Service3Housekeeping.h"

#include "fsfw/datapoollocal/HasLocalDataPoolIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/pus/servicepackets/Service3Packets.h"

Service3Housekeeping::Service3Housekeeping(object_id_t objectId, uint16_t apid, uint8_t serviceId)
    : CommandingServiceBase(objectId, apid, "PUS 3 HK", serviceId, NUM_OF_PARALLEL_COMMANDS,
                            COMMAND_TIMEOUT_SECONDS) {}

Service3Housekeeping::~Service3Housekeeping() {}

ReturnValue_t Service3Housekeeping::isValidSubservice(uint8_t subservice) {
  switch (static_cast<Subservice>(subservice)) {
    case Subservice::ENABLE_PERIODIC_HK_REPORT_GENERATION:
    case Subservice::DISABLE_PERIODIC_HK_REPORT_GENERATION:
    case Subservice::ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
    case Subservice::DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
    case Subservice::REPORT_HK_REPORT_STRUCTURES:
    case Subservice::REPORT_DIAGNOSTICS_REPORT_STRUCTURES:
    case Subservice::GENERATE_ONE_PARAMETER_REPORT:
    case Subservice::GENERATE_ONE_DIAGNOSTICS_REPORT:
    case Subservice::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL:
    case Subservice::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL:
      return returnvalue::OK;
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
                                                             const uint8_t* tcData,
                                                             size_t tcDataLen, MessageQueueId_t* id,
                                                             object_id_t* objectId) {
  ReturnValue_t result = checkAndAcquireTargetID(objectId, tcData, tcDataLen);
  if (result != returnvalue::OK) {
    return result;
  }
  return checkInterfaceAndAcquireMessageQueue(id, objectId);
}

ReturnValue_t Service3Housekeeping::checkAndAcquireTargetID(object_id_t* objectIdToSet,
                                                            const uint8_t* tcData,
                                                            size_t tcDataLen) {
  if (SerializeAdapter::deSerialize(objectIdToSet, &tcData, &tcDataLen,
                                    SerializeIF::Endianness::BIG) != returnvalue::OK) {
    return CommandingServiceBase::INVALID_TC;
  }
  return returnvalue::OK;
}

ReturnValue_t Service3Housekeeping::checkInterfaceAndAcquireMessageQueue(
    MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
  // check HasLocalDataPoolIF property of target
  HasLocalDataPoolIF* possibleTarget =
      ObjectManager::instance()->get<HasLocalDataPoolIF>(*objectId);
  if (possibleTarget == nullptr) {
    return CommandingServiceBase::INVALID_OBJECT;
  }
  *messageQueueToSet = possibleTarget->getCommandQueue();
  return returnvalue::OK;
}

ReturnValue_t Service3Housekeeping::prepareCommand(CommandMessage* message, uint8_t subservice,
                                                   const uint8_t* tcData, size_t tcDataLen,
                                                   uint32_t* state, object_id_t objectId) {
  switch (static_cast<Subservice>(subservice)) {
    case Subservice::ENABLE_PERIODIC_HK_REPORT_GENERATION:
      return prepareReportingTogglingCommand(message, objectId, true, false, tcData, tcDataLen);
    case Subservice::DISABLE_PERIODIC_HK_REPORT_GENERATION:
      return prepareReportingTogglingCommand(message, objectId, false, false, tcData, tcDataLen);
    case Subservice::ENABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
      return prepareReportingTogglingCommand(message, objectId, true, true, tcData, tcDataLen);
    case Subservice::DISABLE_PERIODIC_DIAGNOSTICS_REPORT_GENERATION:
      return prepareReportingTogglingCommand(message, objectId, false, true, tcData, tcDataLen);
    case Subservice::REPORT_HK_REPORT_STRUCTURES:
      return prepareStructureReportingCommand(message, objectId, false, tcData, tcDataLen);
    case Subservice::REPORT_DIAGNOSTICS_REPORT_STRUCTURES:
      return prepareStructureReportingCommand(message, objectId, true, tcData, tcDataLen);
    case Subservice::GENERATE_ONE_PARAMETER_REPORT:
      return prepareOneShotReportCommand(message, objectId, false, tcData, tcDataLen);
    case Subservice::GENERATE_ONE_DIAGNOSTICS_REPORT:
      return prepareOneShotReportCommand(message, objectId, true, tcData, tcDataLen);
    case Subservice::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL:
      return prepareCollectionIntervalModificationCommand(message, objectId, false, tcData,
                                                          tcDataLen);
    case Subservice::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL:
      return prepareCollectionIntervalModificationCommand(message, objectId, true, tcData,
                                                          tcDataLen);
    case Subservice::HK_DEFINITIONS_REPORT:
    case Subservice::DIAGNOSTICS_DEFINITION_REPORT:
    case Subservice::HK_REPORT:
    case Subservice::DIAGNOSTICS_REPORT:
      // Those are telemetry packets.
      return CommandingServiceBase::INVALID_TC;
    default:
      // should never happen, subservice was already checked.
      return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

ReturnValue_t Service3Housekeeping::prepareReportingTogglingCommand(
    CommandMessage* command, object_id_t objectId, bool enableReporting, bool isDiagnostics,
    const uint8_t* tcData, size_t tcDataLen) {
  if (tcDataLen < sizeof(sid_t)) {
    // TC data should consist of object ID and set ID.
    return CommandingServiceBase::INVALID_TC;
  }

  sid_t targetSid = buildSid(objectId, &tcData, &tcDataLen);
  HousekeepingMessage::setToggleReportingCommand(command, targetSid, enableReporting,
                                                 isDiagnostics);
  return returnvalue::OK;
}

ReturnValue_t Service3Housekeeping::prepareStructureReportingCommand(CommandMessage* command,
                                                                     object_id_t objectId,
                                                                     bool isDiagnostics,
                                                                     const uint8_t* tcData,
                                                                     size_t tcDataLen) {
  if (tcDataLen < sizeof(sid_t)) {
    // TC data should consist of object ID and set ID.
    return CommandingServiceBase::INVALID_TC;
  }

  sid_t targetSid = buildSid(objectId, &tcData, &tcDataLen);
  HousekeepingMessage::setStructureReportingCommand(command, targetSid, isDiagnostics);
  return returnvalue::OK;
}

ReturnValue_t Service3Housekeeping::prepareOneShotReportCommand(CommandMessage* command,
                                                                object_id_t objectId,
                                                                bool isDiagnostics,
                                                                const uint8_t* tcData,
                                                                size_t tcDataLen) {
  if (tcDataLen < sizeof(sid_t)) {
    // TC data should consist of object ID and set ID.
    return CommandingServiceBase::INVALID_TC;
  }

  sid_t targetSid = buildSid(objectId, &tcData, &tcDataLen);
  HousekeepingMessage::setOneShotReportCommand(command, targetSid, isDiagnostics);
  return returnvalue::OK;
}

ReturnValue_t Service3Housekeeping::prepareCollectionIntervalModificationCommand(
    CommandMessage* command, object_id_t objectId, bool isDiagnostics, const uint8_t* tcData,
    size_t tcDataLen) {
  if (tcDataLen < sizeof(sid_t) + sizeof(float)) {
    /* SID plus the size of the new collection interval. */
    return CommandingServiceBase::INVALID_TC;
  }

  sid_t targetSid = buildSid(objectId, &tcData, &tcDataLen);
  float newCollectionInterval = 0;
  SerializeAdapter::deSerialize(&newCollectionInterval, &tcData, &tcDataLen,
                                SerializeIF::Endianness::BIG);
  HousekeepingMessage::setCollectionIntervalModificationCommand(
      command, targetSid, newCollectionInterval, isDiagnostics);
  return returnvalue::OK;
}

ReturnValue_t Service3Housekeeping::handleReply(const CommandMessage* reply,
                                                Command_t previousCommand, uint32_t* state,
                                                CommandMessage* optionalNextCommand,
                                                object_id_t objectId, bool* isStep) {
  Command_t command = reply->getCommand();
  switch (command) {
    case (HousekeepingMessage::HK_REPORT): {
      ReturnValue_t result = generateHkReply(reply, static_cast<uint8_t>(Subservice::HK_REPORT));
      if (result != returnvalue::OK) {
        return result;
      }
      return CommandingServiceBase::EXECUTION_COMPLETE;
    }

    case (HousekeepingMessage::DIAGNOSTICS_REPORT): {
      ReturnValue_t result =
          generateHkReply(reply, static_cast<uint8_t>(Subservice::DIAGNOSTICS_REPORT));
      if (result != returnvalue::OK) {
        return result;
      }
      return CommandingServiceBase::EXECUTION_COMPLETE;
    }

    case (HousekeepingMessage::HK_DEFINITIONS_REPORT): {
      return generateHkReply(reply, static_cast<uint8_t>(Subservice::HK_DEFINITIONS_REPORT));
      break;
    }
    case (HousekeepingMessage::DIAGNOSTICS_DEFINITION_REPORT): {
      return generateHkReply(reply,
                             static_cast<uint8_t>(Subservice::DIAGNOSTICS_DEFINITION_REPORT));
      break;
    }

    case (HousekeepingMessage::HK_REQUEST_SUCCESS): {
      return CommandingServiceBase::EXECUTION_COMPLETE;
    }

    case (HousekeepingMessage::HK_REQUEST_FAILURE): {
      failureParameter1 = objectId;
      ReturnValue_t error = returnvalue::FAILED;
      HousekeepingMessage::getHkRequestFailureReply(reply, &error);
      failureParameter2 = error;
      return CommandingServiceBase::EXECUTION_COMPLETE;
    }

    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "Service3Housekeeping::handleReply: Invalid reply with "
                   << "reply command " << command << "!" << std::endl;
#else
      sif::printWarning(
          "Service3Housekeeping::handleReply: Invalid reply with "
          "reply command %hu!\n",
          command);
#endif
      return CommandingServiceBase::INVALID_REPLY;
  }
  return returnvalue::OK;
}

void Service3Housekeeping::handleUnrequestedReply(CommandMessage* reply) {
  ReturnValue_t result = returnvalue::OK;
  Command_t command = reply->getCommand();

  switch (command) {
    case (HousekeepingMessage::DIAGNOSTICS_REPORT): {
      result = generateHkReply(reply, static_cast<uint8_t>(Subservice::DIAGNOSTICS_REPORT));
      break;
    }

    case (HousekeepingMessage::HK_REPORT): {
      result = generateHkReply(reply, static_cast<uint8_t>(Subservice::HK_REPORT));
      break;
    }

    case (HousekeepingMessage::HK_REQUEST_SUCCESS): {
      break;
    }

    case (HousekeepingMessage::HK_REQUEST_FAILURE): {
      break;
    }

    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::warning << "Service3Housekeeping::handleUnrequestedReply: Invalid reply with reply "
                      "command "
                   << command << "!" << std::endl;
#else
      sif::printWarning(
          "Service3Housekeeping::handleUnrequestedReply: Invalid reply with "
          "reply command %hu!\n",
          command);
#endif
      return;
    }
  }

  if (result != returnvalue::OK) {
    /* Configuration error */
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Service3Housekeeping::handleUnrequestedReply: Could not generate reply!"
                 << std::endl;
#else
    sif::printWarning(
        "Service3Housekeeping::handleUnrequestedReply: "
        "Could not generate reply!\n");
#endif
  }
}

MessageQueueId_t Service3Housekeeping::getHkQueue() const { return commandQueue->getId(); }

ReturnValue_t Service3Housekeeping::generateHkReply(const CommandMessage* hkMessage,
                                                    uint8_t subserviceId) {
  store_address_t storeId;

  sid_t sid = HousekeepingMessage::getHkDataReply(hkMessage, &storeId);
  auto resultPair = ipcStore->getData(storeId);
  if (resultPair.first != returnvalue::OK) {
    return resultPair.first;
  }

  HkPacket hkPacket(sid, resultPair.second.data(), resultPair.second.size());
  return sendTmPacket(static_cast<uint8_t>(subserviceId), hkPacket.hkData, hkPacket.hkSize);
}

sid_t Service3Housekeeping::buildSid(object_id_t objectId, const uint8_t** tcData,
                                     size_t* tcDataLen) {
  sid_t targetSid;
  targetSid.objectId = objectId;
  // skip deserialization of object ID, was already done.
  *tcData += sizeof(object_id_t);
  *tcDataLen -= sizeof(object_id_t);
  // size check is expected to be performed beforehand!
  SerializeAdapter::deSerialize(&targetSid.ownerSetId, tcData, tcDataLen,
                                SerializeIF::Endianness::BIG);
  return targetSid;
}
