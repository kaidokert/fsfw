#ifndef FSFW_CFDP_CFDPMESSAGE_H_
#define FSFW_CFDP_CFDPMESSAGE_H_

#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

class CFDPMessage {
 private:
  CFDPMessage();

 public:
  static const uint8_t MESSAGE_ID = messagetypes::CFDP;

  virtual ~CFDPMessage();
  static void setCommand(CommandMessage* message, store_address_t cfdpPacket);

  static store_address_t getStoreId(const CommandMessage* message);

  static void clear(CommandMessage* message);
};

#endif /* FSFW_CFDP_CFDPMESSAGE_H_ */
