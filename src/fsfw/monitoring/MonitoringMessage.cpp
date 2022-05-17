#include "fsfw/monitoring/MonitoringMessage.h"

#include "fsfw/objectmanager/ObjectManager.h"

MonitoringMessage::~MonitoringMessage() {}

void MonitoringMessage::setLimitViolationReport(CommandMessage* message, store_address_t storeId) {
  setTypicalMessage(message, LIMIT_VIOLATION_REPORT, storeId);
}

void MonitoringMessage::setTypicalMessage(CommandMessage* message, Command_t type,
                                          store_address_t storeId) {
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
    case MonitoringMessage::LIMIT_VIOLATION_REPORT: {
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
