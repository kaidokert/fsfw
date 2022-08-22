#include "fsfw/pus/Service20ParameterManagement.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/parameters/HasParametersIF.h"
#include "fsfw/parameters/ParameterMessage.h"
#include "fsfw/parameters/ReceivesParameterMessagesIF.h"
#include "fsfw/pus/servicepackets/Service20Packets.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

Service20ParameterManagement::Service20ParameterManagement(object_id_t objectId, uint16_t apid,
                                                           uint8_t serviceId,
                                                           uint8_t numberOfParallelCommands,
                                                           uint16_t commandTimeoutSeconds)
    : CommandingServiceBase(objectId, apid, "PUS 20 Parameter MGMT", serviceId,
                            numberOfParallelCommands, commandTimeoutSeconds) {}

Service20ParameterManagement::~Service20ParameterManagement() = default;

ReturnValue_t Service20ParameterManagement::isValidSubservice(uint8_t subservice) {
  switch (static_cast<Subservice>(subservice)) {
    case Subservice::PARAMETER_LOAD:
    case Subservice::PARAMETER_DUMP:
      return returnvalue::OK;
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Invalid Subservice for Service 20" << std::endl;
#else
      sif::printError("Invalid Subservice for Service 20\n");
#endif
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

ReturnValue_t Service20ParameterManagement::getMessageQueueAndObject(uint8_t subservice,
                                                                     const uint8_t* tcData,
                                                                     size_t tcDataLen,
                                                                     MessageQueueId_t* id,
                                                                     object_id_t* objectId) {
  ReturnValue_t result = checkAndAcquireTargetID(objectId, tcData, tcDataLen);
  if (result != returnvalue::OK) {
    return result;
  }
  return checkInterfaceAndAcquireMessageQueue(id, objectId);
}

ReturnValue_t Service20ParameterManagement::checkAndAcquireTargetID(object_id_t* objectIdToSet,
                                                                    const uint8_t* tcData,
                                                                    size_t tcDataLen) {
  if (SerializeAdapter::deSerialize(objectIdToSet, &tcData, &tcDataLen,
                                    SerializeIF::Endianness::BIG) != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service20ParameterManagement::checkAndAcquireTargetID: "
               << "Invalid data." << std::endl;
#else
    sif::printError(
        "Service20ParameterManagement::"
        "checkAndAcquireTargetID: Invalid data.\n");
#endif
    return CommandingServiceBase::INVALID_TC;
  }
  return returnvalue::OK;
}

ReturnValue_t Service20ParameterManagement::checkInterfaceAndAcquireMessageQueue(
    MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
  // check ReceivesParameterMessagesIF property of target
  auto* possibleTarget = ObjectManager::instance()->get<ReceivesParameterMessagesIF>(*objectId);
  if (possibleTarget == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service20ParameterManagement::checkInterfaceAndAcquire"
               << "MessageQueue: Can't access object" << std::endl;
    sif::error << "Object ID: " << std::hex << objectId << std::dec << std::endl;
    sif::error << "Make sure it implements ReceivesParameterMessagesIF!" << std::endl;
#else
    sif::printError(
        "Service20ParameterManagement::checkInterfaceAndAcquire"
        "MessageQueue: Can't access object\n");
    sif::printError("Object ID: 0x%08x\n", *objectId);
    sif::printError("Make sure it implements ReceivesParameterMessagesIF!\n");
#endif

    return CommandingServiceBase::INVALID_OBJECT;
  }
  *messageQueueToSet = possibleTarget->getCommandQueue();
  return returnvalue::OK;
}

ReturnValue_t Service20ParameterManagement::prepareCommand(CommandMessage* message,
                                                           uint8_t subservice,
                                                           const uint8_t* tcData, size_t tcDataLen,
                                                           uint32_t* state, object_id_t objectId) {
  switch (static_cast<Subservice>(subservice)) {
    case Subservice::PARAMETER_DUMP: {
      return prepareDumpCommand(message, tcData, tcDataLen);
    } break;
    case Subservice::PARAMETER_LOAD: {
      return prepareLoadCommand(message, tcData, tcDataLen);
    } break;
    default:
      return returnvalue::FAILED;
  }
}

ReturnValue_t Service20ParameterManagement::prepareDumpCommand(CommandMessage* message,
                                                               const uint8_t* tcData,
                                                               size_t tcDataLen) {
  /* the first part is the objectId, but we have extracted that earlier
  and only need the parameterId */
  tcData += sizeof(object_id_t);
  tcDataLen -= sizeof(object_id_t);
  ParameterId_t parameterId;
  if (SerializeAdapter::deSerialize(&parameterId, &tcData, &tcDataLen,
                                    SerializeIF::Endianness::BIG) != returnvalue::OK) {
    return CommandingServiceBase::INVALID_TC;
  }
  /* The length should have been decremented to 0 by this point */
  if (tcDataLen != 0) {
    return CommandingServiceBase::INVALID_TC;
  }

  ParameterMessage::setParameterDumpCommand(message, parameterId);
  return returnvalue::OK;
}

ReturnValue_t Service20ParameterManagement::prepareLoadCommand(CommandMessage* message,
                                                               const uint8_t* tcData,
                                                               size_t tcDataLen) {
  if (tcDataLen < sizeof(object_id_t) + sizeof(ParameterId_t) + sizeof(uint32_t)) {
    return CommandingServiceBase::INVALID_TC;
  }

  uint8_t* storePointer = nullptr;
  store_address_t storeAddress;
  size_t parameterDataLen =
      tcDataLen - sizeof(object_id_t) - sizeof(ParameterId_t) - sizeof(uint32_t);
  if (parameterDataLen == 0) {
    return CommandingServiceBase::INVALID_TC;
  }
  ReturnValue_t result = ipcStore->getFreeElement(&storeAddress, parameterDataLen, &storePointer);
  if (result != returnvalue::OK) {
    return result;
  }

  /* Following format is expected: The first 4 bytes in the TC data are the 4 byte
  parameter ID (ParameterId_t). The second 4 bytes are the parameter information field,
  containing the following 1 byte fields:
   1. ECSS PTC field
   2. ECSS PFC field
   3. Number of rows
   4. Number of columns */
  ParameterLoadCommand command(storePointer, parameterDataLen);
  result = command.deSerialize(&tcData, &tcDataLen, SerializeIF::Endianness::BIG);
  if (result != returnvalue::OK) {
    return result;
  }

  ParameterMessage::setParameterLoadCommand(message, command.getParameterId(), storeAddress,
                                            command.getPtc(), command.getPfc(), command.getRows(),
                                            command.getColumns());
  return returnvalue::OK;
}

ReturnValue_t Service20ParameterManagement::handleReply(const CommandMessage* reply,
                                                        Command_t previousCommand, uint32_t* state,
                                                        CommandMessage* optionalNextCommand,
                                                        object_id_t objectId, bool* isStep) {
  Command_t replyId = reply->getCommand();

  switch (replyId) {
    case ParameterMessage::REPLY_PARAMETER_DUMP: {
      ConstAccessorPair parameterData = ipcStore->getData(ParameterMessage::getStoreId(reply));
      if (parameterData.first != returnvalue::OK) {
        return returnvalue::FAILED;
      }

      ParameterId_t parameterId = ParameterMessage::getParameterId(reply);
      ParameterDumpReply parameterReply(objectId, parameterId, parameterData.second.data(),
                                        parameterData.second.size());
      return sendTmPacket(static_cast<uint8_t>(Subservice::PARAMETER_DUMP_REPLY), parameterReply);
    }
    default:
      return CommandingServiceBase::INVALID_REPLY;
  }
}
