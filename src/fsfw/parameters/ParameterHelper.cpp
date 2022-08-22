#include "fsfw/parameters/ParameterHelper.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/parameters/ParameterMessage.h"

ParameterHelper::ParameterHelper(ReceivesParameterMessagesIF* owner) : owner(owner) {}

ParameterHelper::~ParameterHelper() {}

ReturnValue_t ParameterHelper::handleParameterMessage(CommandMessage* message) {
  if (storage == nullptr) {
    // ParameterHelper was not initialized
    return returnvalue::FAILED;
  }

  ReturnValue_t result = returnvalue::FAILED;
  switch (message->getCommand()) {
    case ParameterMessage::CMD_PARAMETER_DUMP: {
      ParameterWrapper description;
      uint8_t domain = HasParametersIF::getDomain(ParameterMessage::getParameterId(message));
      uint8_t uniqueIdentifier =
          HasParametersIF::getUniqueIdentifierId(ParameterMessage::getParameterId(message));
      result = owner->getParameter(domain, uniqueIdentifier, &description, &description, 0);
      if (result == returnvalue::OK) {
        result = sendParameter(message->getSender(), ParameterMessage::getParameterId(message),
                               &description);
      }
    } break;
    case ParameterMessage::CMD_PARAMETER_LOAD: {
      ParameterId_t parameterId = 0;
      uint8_t ptc = 0;
      uint8_t pfc = 0;
      uint8_t rows = 0;
      uint8_t columns = 0;
      store_address_t storeId = ParameterMessage::getParameterLoadCommand(
          message, &parameterId, &ptc, &pfc, &rows, &columns);
      Type type(Type::getActualType(ptc, pfc));

      uint8_t domain = HasParametersIF::getDomain(parameterId);
      uint8_t uniqueIdentifier = HasParametersIF::getUniqueIdentifierId(parameterId);
      uint16_t linearIndex = HasParametersIF::getIndex(parameterId);

      ConstStorageAccessor accessor(storeId);
      result = storage->getData(storeId, accessor);
      if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "ParameterHelper::handleParameterMessage: Getting"
                   << " store data failed for load command." << std::endl;
#endif
        break;
      }

      ParameterWrapper streamWrapper;
      result = streamWrapper.set(type, rows, columns, accessor.data(), accessor.size());
      if (result != returnvalue::OK) {
        return result;
      }

      ParameterWrapper ownerWrapper;
      result =
          owner->getParameter(domain, uniqueIdentifier, &ownerWrapper, &streamWrapper, linearIndex);
      if (result != returnvalue::OK) {
        return result;
      }

      result = ownerWrapper.copyFrom(&streamWrapper, linearIndex);
      if (result != returnvalue::OK) {
        return result;
      }

      result = sendParameter(message->getSender(), ParameterMessage::getParameterId(message),
                             &ownerWrapper);
      break;
    }
    default:
      return returnvalue::FAILED;
  }

  if (result != returnvalue::OK) {
    rejectCommand(message->getSender(), result, message->getCommand());
  }

  return returnvalue::OK;
}

ReturnValue_t ParameterHelper::sendParameter(MessageQueueId_t to, uint32_t id,
                                             const ParameterWrapper* description) {
  size_t serializedSize = description->getSerializedSize();

  uint8_t* storeElement = nullptr;
  store_address_t address;

  ReturnValue_t result = storage->getFreeElement(&address, serializedSize, &storeElement);
  if (result != returnvalue::OK) {
    return result;
  }

  size_t storeElementSize = 0;

  result = description->serialize(&storeElement, &storeElementSize, serializedSize,
                                  SerializeIF::Endianness::BIG);

  if (result != returnvalue::OK) {
    storage->deleteData(address);
    return result;
  }

  CommandMessage reply;

  ParameterMessage::setParameterDumpReply(&reply, id, address);

  MessageQueueSenderIF::sendMessage(to, &reply, ownerQueueId);

  return returnvalue::OK;
}

ReturnValue_t ParameterHelper::initialize() {
  ownerQueueId = owner->getCommandQueue();

  storage = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  if (storage == nullptr) {
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  return returnvalue::OK;
}

void ParameterHelper::rejectCommand(MessageQueueId_t to, ReturnValue_t reason,
                                    Command_t initialCommand) {
  CommandMessage reply;
  reply.setReplyRejected(reason, initialCommand);
  MessageQueueSenderIF::sendMessage(to, &reply, ownerQueueId);
}
