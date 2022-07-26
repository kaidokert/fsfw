#include <catch2/catch_test_macros.hpp>

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcservices/TmSendHelper.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"
#include "mocks/CdsShortTimestamperMock.h"

TEST_CASE("TM Store And Send Helper", "[tm-store-send-helper]") {
  auto timeStamper = CdsShortTimestamperMock();
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto storeHelper = TmStoreHelper(2, pool, timeStamper);

  MessageQueueId_t destId = 1;
  auto errReporter = InternalErrorReporterMock();
  auto msgQueue = MessageQueueMock();
  msgQueue.setDefaultDestination(destId);
  TmSendHelper sendHelper(msgQueue, errReporter, destId);
}
