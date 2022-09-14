#include "fsfw/pus/Service2DeviceAccess.h"

#include "fsfw/action/ActionMessage.h"
#include "fsfw/devicehandlers/DeviceHandlerIF.h"
#include "fsfw/devicehandlers/DeviceHandlerMessage.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/pus/servicepackets/Service2Packets.h"
#include "fsfw/serialize/EndianConverter.h"
#include "fsfw/serialize/SerialLinkedListAdapter.h"
#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

Service2DeviceAccess::Service2DeviceAccess(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                                           uint8_t numberOfParallelCommands,
                                           uint16_t commandTimeoutSeconds)
    : CommandingServiceBase(objectId, apid, "PUS 2 Raw Commanding", serviceId,
                            numberOfParallelCommands, commandTimeoutSeconds) {}

Service2DeviceAccess::~Service2DeviceAccess() {}

ReturnValue_t Service2DeviceAccess::isValidSubservice(uint8_t subservice) {
  switch (static_cast<Subservice>(subservice)) {
    case Subservice::COMMAND_RAW_COMMANDING:
    case Subservice::COMMAND_TOGGLE_WIRETAPPING:
      return returnvalue::OK;
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Invalid Subservice" << std::endl;
#endif
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

ReturnValue_t Service2DeviceAccess::getMessageQueueAndObject(uint8_t subservice,
                                                             const uint8_t* tcData,
                                                             size_t tcDataLen, MessageQueueId_t* id,
                                                             object_id_t* objectId) {
  if (tcDataLen < sizeof(object_id_t)) {
    return CommandingServiceBase::INVALID_TC;
  }
  SerializeAdapter::deSerialize(objectId, &tcData, &tcDataLen, SerializeIF::Endianness::BIG);

  return checkInterfaceAndAcquireMessageQueue(id, objectId);
}

ReturnValue_t Service2DeviceAccess::checkInterfaceAndAcquireMessageQueue(
    MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
  DeviceHandlerIF* possibleTarget = ObjectManager::instance()->get<DeviceHandlerIF>(*objectId);
  if (possibleTarget == nullptr) {
    return CommandingServiceBase::INVALID_OBJECT;
  }
  *messageQueueToSet = possibleTarget->getCommandQueue();
  return returnvalue::OK;
}

ReturnValue_t Service2DeviceAccess::prepareCommand(CommandMessage* message, uint8_t subservice,
                                                   const uint8_t* tcData, size_t tcDataLen,
                                                   uint32_t* state, object_id_t objectId) {
  switch (static_cast<Subservice>(subservice)) {
    case Subservice::COMMAND_RAW_COMMANDING: {
      return prepareRawCommand(message, tcData, tcDataLen);
    } break;
    case Subservice::COMMAND_TOGGLE_WIRETAPPING: {
      return prepareWiretappingCommand(message, tcData, tcDataLen);
    } break;
    default:
      return returnvalue::FAILED;
  }
}

ReturnValue_t Service2DeviceAccess::prepareRawCommand(CommandMessage* messageToSet,
                                                      const uint8_t* tcData, size_t tcDataLen) {
  RawCommand RawCommand(tcData, tcDataLen);
  // store command into the Inter Process Communication Store
  store_address_t storeAddress;
  ReturnValue_t result =
      ipcStore->addData(&storeAddress, RawCommand.getCommand(), RawCommand.getCommandSize());
  DeviceHandlerMessage::setDeviceHandlerRawCommandMessage(messageToSet, storeAddress);
  return result;
}

ReturnValue_t Service2DeviceAccess::prepareWiretappingCommand(CommandMessage* messageToSet,
                                                              const uint8_t* tcData,
                                                              size_t tcDataLen) {
  if (tcDataLen != WiretappingToggle::WIRETAPPING_COMMAND_SIZE) {
    return CommandingServiceBase::INVALID_TC;
  }
  WiretappingToggle command;
  ReturnValue_t result = command.deSerialize(&tcData, &tcDataLen, SerializeIF::Endianness::BIG);
  DeviceHandlerMessage::setDeviceHandlerWiretappingMessage(messageToSet,
                                                           command.getWiretappingMode());
  return result;
}

ReturnValue_t Service2DeviceAccess::handleReply(const CommandMessage* reply,
                                                Command_t previousCommand, uint32_t* state,
                                                CommandMessage* optionalNextCommand,
                                                object_id_t objectId, bool* isStep) {
  switch (reply->getCommand()) {
    case CommandMessage::REPLY_COMMAND_OK:
      return returnvalue::OK;
    case CommandMessage::REPLY_REJECTED:
      return reply->getReplyRejectedReason();
    default:
      return CommandingServiceBase::INVALID_REPLY;
  }
}

// All device handlers set service 2 as default raw receiver for wiretapping
// so we have to handle those unrequested messages.
void Service2DeviceAccess::handleUnrequestedReply(CommandMessage* reply) {
  switch (reply->getCommand()) {
    case DeviceHandlerMessage::REPLY_RAW_COMMAND:
      sendWiretappingTm(reply, static_cast<uint8_t>(Subservice::REPLY_WIRETAPPING_RAW_TC));
      break;
    case DeviceHandlerMessage::REPLY_RAW_REPLY:
      sendWiretappingTm(reply, static_cast<uint8_t>(Subservice::REPLY_RAW));
      break;
    default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Unknown message in Service2DeviceAccess::"
                    "handleUnrequestedReply with command ID "
                 << reply->getCommand() << std::endl;
#endif
      break;
  }
  // Must be reached by all cases to clear message
  reply->clear();
}

void Service2DeviceAccess::sendWiretappingTm(CommandMessage* reply, uint8_t subservice) {
  // Raw Wiretapping
  // Get Address of Data from Message
  store_address_t storeAddress = DeviceHandlerMessage::getStoreAddress(reply);
  const uint8_t* data = nullptr;
  size_t size = 0;
  ReturnValue_t result = ipcStore->getData(storeAddress, &data, &size);
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service2DeviceAccess::sendWiretappingTm: Data Lost in "
                  "handleUnrequestedReply with failure ID "
               << result << std::endl;
#endif
    return;
  }

  // Init our dummy packet and correct endianness of object ID before
  // sending it back.
  WiretappingPacket tmPacket(DeviceHandlerMessage::getDeviceObjectId(reply), data);
  result = sendTmPacket(subservice, tmPacket.objectId, tmPacket.data, size);
  if (result != returnvalue::OK) {
    // TODO: Warning
    return;
  }
}

MessageQueueId_t Service2DeviceAccess::getDeviceQueue() { return commandQueue->getId(); }
