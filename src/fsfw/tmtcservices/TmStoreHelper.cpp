#include "TmStoreHelper.h"

#include "TmTcMessage.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"

TmStoreHelper::TmStoreHelper(StorageManagerIF *tmStore, MessageQueueId_t tmtcMsgDest,
                             MessageQueueId_t tmtcMsgSrc, InternalErrorReporterIF *reporter)
    : creator(params),
      tmtcMsgDest(tmtcMsgDest),
      tmtcMsgSrc(tmtcMsgSrc),
      errReporter(reporter),
      tmStore(tmStore) {}

void TmStoreHelper::preparePacket(uint16_t apid, uint8_t service, uint8_t subservice,
                                  uint16_t counter) {
  // TODO: Implement
  // creator.setApid(apid);
  params.service = service;
  params.subservice = subservice;
  params.messageTypeCounter = counter;
  // TODO: Implement serialize and then serialize into the store
}

ReturnValue_t TmStoreHelper::sendPacket() {
  TmTcMessage tmMessage(currentAddr);
  ReturnValue_t result = MessageQueueSenderIF::sendMessage(tmtcMsgDest, &tmMessage, tmtcMsgSrc);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    tmStore->deleteData(currentAddr);
    if (errReporter != nullptr) {
      errReporter->lostTm();
    }
    return result;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

void TmStoreHelper::setMsgDestination(MessageQueueId_t msgDest) { tmtcMsgDest = msgDest; }

void TmStoreHelper::setMsgSource(MessageQueueId_t msgSrc) { tmtcMsgSrc = msgSrc; }

void TmStoreHelper::setInternalErrorReporter(InternalErrorReporterIF *reporter) {
  errReporter = reporter;
}
