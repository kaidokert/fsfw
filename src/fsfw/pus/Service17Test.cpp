#include "fsfw/pus/Service17Test.h"

#include "fsfw/FSFW.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcpacket/pus/tm/TmPacketStored.h"

Service17Test::Service17Test(object_id_t objectId, uint16_t apid, uint8_t serviceId,
                             StorageManagerIF* tmStore, StorageManagerIF* ipcStore,
                             InternalErrorReporterIF* errReporter)
    : PusServiceBase(objectId, apid, serviceId, ipcStore),
      helper(tmStore, MessageQueueIF::NO_QUEUE, MessageQueueIF::NO_QUEUE, errReporter),
      packetSubCounter(0) {}

Service17Test::~Service17Test() = default;

ReturnValue_t Service17Test::handleRequest(uint8_t subservice) {
  switch (subservice) {
    case Subservice::CONNECTION_TEST: {
      helper.preparePacket(apid, serviceId, Subservice::CONNECTION_TEST_REPORT, packetSubCounter);
      helper.sendPacket();
      return HasReturnvaluesIF::RETURN_OK;
    }
    case Subservice::EVENT_TRIGGER_TEST: {
      helper.preparePacket(apid, serviceId, Subservice::CONNECTION_TEST_REPORT, packetSubCounter++);
      helper.sendPacket();
      triggerEvent(TEST, 1234, 5678);
      return RETURN_OK;
    }
    default:
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

ReturnValue_t Service17Test::performService() { return HasReturnvaluesIF::RETURN_OK; }

ReturnValue_t Service17Test::initialize() {
  ReturnValue_t result = PusServiceBase::initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  helper.setMsgDestination(requestQueue->getDefaultDestination());
  helper.setMsgSource(requestQueue->getId());
  if (tm) }
