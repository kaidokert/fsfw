#include "fsfw/pus/Service17Test.h"

#include "fsfw/FSFW.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tmtcservices/tmHelpers.h"

Service17Test::Service17Test(object_id_t objectId, uint16_t apid, uint8_t serviceId)
    : PusServiceBase(objectId, apid, serviceId),
      storeHelper(apid, nullptr),
      sendHelper(nullptr),
      packetSubCounter(0) {}

Service17Test::~Service17Test() = default;

ReturnValue_t Service17Test::handleRequest(uint8_t subservice) {
  switch (subservice) {
    case Subservice::CONNECTION_TEST: {
      storeHelper.preparePacket(serviceId, Subservice::CONNECTION_TEST_REPORT, packetSubCounter);
      return tm::storeAndSendTmPacket(storeHelper, sendHelper);
    }
    case Subservice::EVENT_TRIGGER_TEST: {
      storeHelper.preparePacket(serviceId, Subservice::CONNECTION_TEST_REPORT, packetSubCounter++);
      triggerEvent(TEST, 1234, 5678);
      return tm::storeAndSendTmPacket(storeHelper, sendHelper);
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
  initializeTmHelpers(sendHelper, storeHelper);
  if (storeHelper.getTmStore() == nullptr) {
    auto* tmStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
    if (tmStore == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    storeHelper.setTmStore(tmStore);
  }
  return result;
}

void Service17Test::setCustomTmStore(StorageManagerIF* tmStore_) {
  storeHelper.setTmStore(tmStore_);
}
