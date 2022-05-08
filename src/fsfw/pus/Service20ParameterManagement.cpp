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
    : CommandingServiceBase(objectId, apid, serviceId, numberOfParallelCommands,
                            commandTimeoutSeconds) {}

Service20ParameterManagement::~Service20ParameterManagement() = default;

ReturnValue_t Service20ParameterManagement::isValidSubservice(uint8_t subservice) {
  switch (static_cast<Subservice>(subservice)) {
    case Subservice::PARAMETER_LOAD:
    case Subservice::PARAMETER_DUMP:
      return HasReturnvaluesIF::RETURN_OK;
    default:
      FSFW_FLOGE("Invalid Subservice {} for Service 20\n", subservice);
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

ReturnValue_t Service20ParameterManagement::getMessageQueueAndObject(uint8_t subservice,
                                                                     const uint8_t* tcData,
                                                                     size_t tcDataLen,
                                                                     MessageQueueId_t* id,
                                                                     object_id_t* objectId) {
  ReturnValue_t result = checkAndAcquireTargetID(objectId, tcData, tcDataLen);
  if (result != RETURN_OK) {
    return result;
  }
  return checkInterfaceAndAcquireMessageQueue(id, objectId);
}

ReturnValue_t Service20ParameterManagement::checkAndAcquireTargetID(object_id_t* objectIdToSet,
                                                                    const uint8_t* tcData,
                                                                    size_t tcDataLen) {
  if (SerializeAdapter::deSerialize(objectIdToSet, &tcData, &tcDataLen,
                                    SerializeIF::Endianness::BIG) != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGE("checkAndAcquireTargetID: Invalid data\n");
    return CommandingServiceBase::INVALID_TC;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service20ParameterManagement::checkInterfaceAndAcquireMessageQueue(
    MessageQueueId_t* messageQueueToSet, const object_id_t* objectId) {
  // check ReceivesParameterMessagesIF property of target
  auto* possibleTarget = ObjectManager::instance()->get<ReceivesParameterMessagesIF>(*objectId);
  if (possibleTarget == nullptr) {
    FSFW_FLOGE(
        "checkInterfaceAndAcquire: Can't retrieve message queue | Object ID {:#08x}\n"
        "Does it implement ReceivesParameterMessagesIF?\n",
        *objectId);
    return CommandingServiceBase::INVALID_OBJECT;
  }
  *messageQueueToSet = possibleTarget->getCommandQueue();
  return HasReturnvaluesIF::RETURN_OK;
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
      return HasReturnvaluesIF::RETURN_FAILED;
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
                                    SerializeIF::Endianness::BIG) != HasReturnvaluesIF::RETURN_OK) {
    return CommandingServiceBase::INVALID_TC;
  }
  /* The length should have been decremented to 0 by this point */
  if (tcDataLen != 0) {
    return CommandingServiceBase::INVALID_TC;
  }

  ParameterMessage::setParameterDumpCommand(message, parameterId);
  return HasReturnvaluesIF::RETURN_OK;
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
  ReturnValue_t result = IPCStore->getFreeElement(&storeAddress, parameterDataLen, &storePointer);
  if (result != HasReturnvaluesIF::RETURN_OK) {
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
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  ParameterMessage::setParameterLoadCommand(message, command.getParameterId(), storeAddress,
                                            command.getPtc(), command.getPfc(), command.getRows(),
                                            command.getColumns());
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Service20ParameterManagement::handleReply(const CommandMessage* reply,
                                                        Command_t previousCommand, uint32_t* state,
                                                        CommandMessage* optionalNextCommand,
                                                        object_id_t objectId, bool* isStep) {
  Command_t replyId = reply->getCommand();

  switch (replyId) {
    case ParameterMessage::REPLY_PARAMETER_DUMP: {
      ConstAccessorPair parameterData = IPCStore->getData(ParameterMessage::getStoreId(reply));
      if (parameterData.first != HasReturnvaluesIF::RETURN_OK) {
        return HasReturnvaluesIF::RETURN_FAILED;
      }

      ParameterId_t parameterId = ParameterMessage::getParameterId(reply);
      ParameterDumpReply parameterReply(objectId, parameterId, parameterData.second.data(),
                                        parameterData.second.size());
      sendTmPacket(static_cast<uint8_t>(Subservice::PARAMETER_DUMP_REPLY), &parameterReply);
      return HasReturnvaluesIF::RETURN_OK;
    }
    default:
      return CommandingServiceBase::INVALID_REPLY;
  }
}
