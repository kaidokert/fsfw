#ifndef MONITORINGMESSAGE_H_
#define MONITORINGMESSAGE_H_

#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "monitoringConf.h"

class MonitoringMessage : public CommandMessage {
 public:
  static const uint8_t MESSAGE_ID = messagetypes::MONITORING;
  // Object id could be useful, but we better manage that on service level (register potential
  // reporters).
  static const Command_t LIMIT_VIOLATION_REPORT = MAKE_COMMAND_ID(10);
  virtual ~MonitoringMessage();
  static void setLimitViolationReport(CommandMessage* message, store_address_t storeId);
  static void clear(CommandMessage* message);
  static store_address_t getStoreId(const CommandMessage* message);
  static void setTypicalMessage(CommandMessage* message, Command_t type, store_address_t storeId);
};

#endif /* MONITORINGMESSAGE_H_ */
