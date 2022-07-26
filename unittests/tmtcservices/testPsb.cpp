#include <catch2/catch_test_macros.hpp>

#include "fsfw/ipc/QueueFactory.h"
#include "mocks/AcceptsTmMock.h"
#include "mocks/MessageQueueMock.h"
#include "mocks/PusServiceBaseMock.h"
#include "mocks/PusVerificationReporterMock.h"

TEST_CASE("Pus Service Base", "[pus-service-base]") {
  auto verificationReporter = PusVerificationReporterMock();
  auto msgQueue = MessageQueueMock(1);
  auto tmReceiver = AcceptsTmMock(2);
  auto psbParams = PsbParams(0, 0x02, 17);
  psbParams.verifReporter = &verificationReporter;
  psbParams.reqQueue = &msgQueue;
  psbParams.tmReceiver = &tmReceiver;
  auto psb = PsbMock(psbParams);
  store_address_t dummyId(1);
  auto reqQueue = psb.getRequestQueue();
  TmTcMessage tmtcMsg(dummyId);
  REQUIRE(psb.initialize() == HasReturnvaluesIF::RETURN_OK);

  SECTION("State") {
    REQUIRE(psb.getIdentifier() == 17);
    REQUIRE(psb.getObjectId() == 0);
  }

  SECTION("Send Request") {
    msgQueue.addReceivedMessage(tmtcMsg);
    REQUIRE(psb.performOperation(0) == retval::OK);
  }
}