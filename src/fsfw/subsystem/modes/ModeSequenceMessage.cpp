#include "fsfw/subsystem/modes/ModeSequenceMessage.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

void ModeSequenceMessage::setModeSequenceMessage(CommandMessage* message, Command_t command,
                                                 Mode_t sequence, store_address_t storeAddress) {
  message->setCommand(command);
  message->setParameter(storeAddress.raw);
  message->setParameter2(sequence);
}

void ModeSequenceMessage::setModeSequenceMessage(CommandMessage* message, Command_t command,
                                                 Mode_t sequence) {
  message->setCommand(command);
  message->setParameter2(sequence);
}

void ModeSequenceMessage::setModeSequenceMessage(CommandMessage* message, Command_t command,
                                                 store_address_t storeAddress) {
  message->setCommand(command);
  message->setParameter(storeAddress.raw);
}

store_address_t ModeSequenceMessage::getStoreAddress(const CommandMessage* message) {
  store_address_t address;
  address.raw = message->getParameter();
  return address;
}

Mode_t ModeSequenceMessage::getSequenceId(const CommandMessage* message) {
  return message->getParameter2();
}

Mode_t ModeSequenceMessage::getTableId(const CommandMessage* message) {
  return message->getParameter2();
}

uint32_t ModeSequenceMessage::getNumber(const CommandMessage* message) {
  return message->getParameter2();
}

void ModeSequenceMessage::clear(CommandMessage* message) {
  switch (message->getCommand()) {
    case ADD_SEQUENCE:
    case ADD_TABLE:
    case SEQUENCE_LIST:
    case TABLE_LIST:
    case TABLE:
    case SEQUENCE: {
      StorageManagerIF* ipcStore =
          ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
      if (ipcStore != nullptr) {
        ipcStore->deleteData(ModeSequenceMessage::getStoreAddress(message));
      }
    }
    /* NO BREAK falls through*/
    case DELETE_SEQUENCE:
    case DELETE_TABLE:
    case READ_SEQUENCE:
    case READ_TABLE:
    case LIST_SEQUENCES:
    case LIST_TABLES:
    case READ_FREE_SEQUENCE_SLOTS:
    case FREE_SEQUENCE_SLOTS:
    case READ_FREE_TABLE_SLOTS:
    case FREE_TABLE_SLOTS:
    default:
      message->setCommand(CommandMessage::CMD_NONE);
      message->setParameter(0);
      message->setParameter2(0);
      break;
  }
}
