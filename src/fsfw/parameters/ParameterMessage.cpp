#include "fsfw/parameters/ParameterMessage.h"

#include "fsfw/objectmanager/ObjectManager.h"

ParameterId_t ParameterMessage::getParameterId(const CommandMessage* message) {
  return message->getParameter();
}

store_address_t ParameterMessage::getStoreId(const CommandMessage* message) {
  store_address_t address;
  address.raw = message->getParameter2();
  return address;
}

void ParameterMessage::setParameterDumpCommand(CommandMessage* message, ParameterId_t id) {
  message->setCommand(CMD_PARAMETER_DUMP);
  message->setParameter(id);
}

void ParameterMessage::setParameterDumpReply(CommandMessage* message, ParameterId_t id,
                                             store_address_t storageID) {
  message->setCommand(REPLY_PARAMETER_DUMP);
  message->setParameter(id);
  message->setParameter2(storageID.raw);
}

void ParameterMessage::setParameterLoadCommand(CommandMessage* message, ParameterId_t id,
                                               store_address_t storeId, uint8_t ptc, uint8_t pfc,
                                               uint8_t rows = 1, uint8_t columns = 1) {
  message->setCommand(CMD_PARAMETER_LOAD);
  message->setParameter(id);
  message->setParameter2(storeId.raw);
  uint32_t packedParameterSettings = (ptc << 24) | (pfc << 16) | (rows << 8) | columns;
  message->setParameter3(packedParameterSettings);
}

store_address_t ParameterMessage::getParameterLoadCommand(const CommandMessage* message,
                                                          ParameterId_t* parameterId, uint8_t* ptc,
                                                          uint8_t* pfc, uint8_t* rows,
                                                          uint8_t* columns) {
  *parameterId = message->getParameter();
  uint32_t packedParamSettings = message->getParameter3();
  *ptc = packedParamSettings >> 24 & 0xff;
  *pfc = packedParamSettings >> 16 & 0xff;
  *rows = packedParamSettings >> 8 & 0xff;
  *columns = packedParamSettings & 0xff;
  return static_cast<store_address_t>(message->getParameter2());
}

void ParameterMessage::clear(CommandMessage* message) {
  switch (message->getCommand()) {
    case CMD_PARAMETER_LOAD:
    case REPLY_PARAMETER_DUMP: {
      StorageManagerIF* ipcStore =
          ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
      if (ipcStore != NULL) {
        ipcStore->deleteData(getStoreId(message));
      }
      break;
    }
    default:
      break;
  }
}
