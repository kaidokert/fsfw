#ifndef FSFW_TMTCSERVICES_STOREHELPER_H
#define FSFW_TMTCSERVICES_STOREHELPER_H

#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/ipc/MessageQueueMessageIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/tmtcpacket/pus/tm/PusTmCreator.h"

// TODO: Serializing a packet into a store and sending the message are two different tasks
//       Move them into separate classes
class TmStoreHelper {
 public:
  TmStoreHelper(StorageManagerIF* tmStore, MessageQueueId_t tmtcMsgDest,
                MessageQueueId_t tmtcMsgSrc, InternalErrorReporterIF* reporter);

  void setInternalErrorReporter(InternalErrorReporterIF* reporter);
  void setMsgDestination(MessageQueueId_t msgDest);
  void setMsgSource(MessageQueueId_t msgSrc);
  void preparePacket(uint16_t apid, uint8_t service, uint8_t subservice, uint16_t counter);
  ReturnValue_t sendPacket();

 private:
  PusTmParams params{};
  PusTmCreator creator;

  bool doErrorReporting = true;
  MessageQueueId_t tmtcMsgDest;
  MessageQueueId_t tmtcMsgSrc;
  InternalErrorReporterIF* errReporter;
  store_address_t currentAddr{};
  StorageManagerIF* tmStore;
};
#endif  // FSFW_TMTCSERVICES_STOREHELPER_H
