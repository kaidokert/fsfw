#include "fsfw/pus/Service17Test.h"

#include "fsfw/FSFW.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tmtcservices/tmHelpers.h"

Service17Test::Service17Test(PsbParams params)
    : PusServiceBase(params),
      storeHelper(params.apid),
      tmHelper(params.serviceId, storeHelper, sendHelper) {
  params.name = "PUS 17 Test";
}

Service17Test::~Service17Test() = default;

ReturnValue_t Service17Test::handleRequest(uint8_t subservice) {
  switch (subservice) {
    case Subservice::CONNECTION_TEST: {
      ReturnValue_t result = tmHelper.prepareTmPacket(Subservice::CONNECTION_TEST_REPORT);
      if (result != returnvalue::OK) {
        return result;
      }
      return tmHelper.storeAndSendTmPacket();
    }
    case Subservice::EVENT_TRIGGER_TEST: {
      triggerEvent(TEST, 1234, 5678);
      ReturnValue_t result = tmHelper.prepareTmPacket(Subservice::EVENT_TRIGGER_TEST);
      if (result != returnvalue::OK) {
        return result;
      }
      return tmHelper.storeAndSendTmPacket();
    }
    default:
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
}

ReturnValue_t Service17Test::performService() { return returnvalue::OK; }

ReturnValue_t Service17Test::initialize() {
  ReturnValue_t result = PusServiceBase::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  initializeTmHelpers(sendHelper, storeHelper);
  if (storeHelper.getTmStore() == nullptr) {
    auto* tmStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TM_STORE);
    if (tmStore == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
    storeHelper.setTmStore(*tmStore);
  }
  return result;
}

void Service17Test::setCustomTmStore(StorageManagerIF& tmStore_) {
  storeHelper.setTmStore(tmStore_);
}
