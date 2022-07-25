#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcservices/TmSendHelper.h"
#include "mocks/InternalErrorReporterMock.h"
#include "mocks/MessageQueueMockBase.h"

TEST_CASE("TM Send Helper", "[tm-send-helper]") {
  auto errReporter = InternalErrorReporterMock();
  auto msgQueue = MessageQueueMockBase();
  TmSendHelper sendHelper(&msgQueue, &errReporter);

  SECTION("State") {

  }
}