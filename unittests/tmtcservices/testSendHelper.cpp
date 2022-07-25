#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "fsfw/tmtcservices/TmSendHelper.h"
#include "mocks/InternalErrorReporterMock.h"
#include "mocks/MessageQueueMock.h"

TEST_CASE("TM Send Helper", "[tm-send-helper]") {
  auto errReporter = InternalErrorReporterMock();
  auto msgQueue = MessageQueueMock();
  TmSendHelper sendHelper(&msgQueue, &errReporter);

  SECTION("State") {
  }
}