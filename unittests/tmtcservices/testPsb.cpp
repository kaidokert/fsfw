#include <catch2/catch_test_macros.hpp>

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/storagemanager/LocalPool.h"
#include "mocks/AcceptsTmMock.h"
#include "mocks/CdsShortTimestamperMock.h"
#include "mocks/MessageQueueMock.h"
#include "mocks/PusServiceBaseMock.h"
#include "mocks/PusVerificationReporterMock.h"

TEST_CASE("Pus Service Base", "[pus-service-base]") {
  auto verificationReporter = PusVerificationReporterMock();
  auto msgQueue = MessageQueueMock(1);
  auto tmReceiver = AcceptsTmMock(2);
  auto psbParams = PsbParams(0, 0x02, 17);

  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);

  psbParams.verifReporter = &verificationReporter;
  psbParams.reqQueue = &msgQueue;
  psbParams.tmReceiver = &tmReceiver;
  psbParams.tcPool = &pool;
  auto psb = PsbMock(psbParams);

  store_address_t storeId;
  TmTcMessage tmtcMsg;

  // Components to create valid PUS packets
  auto packetId = PacketId(ccsds::PacketType::TC, true, 0x02);
  auto spParams =
      SpacePacketParams(packetId, PacketSeqCtrl(ccsds::SequenceFlags::UNSEGMENTED, 0x34), 0x00);
  auto pusParams = PusTcParams(17, 1);
  PusTcCreator creator(spParams, pusParams);

  REQUIRE(psb.initialize() == HasReturnvaluesIF::RETURN_OK);

  SECTION("State") {
    REQUIRE(psb.getIdentifier() == 17);
    REQUIRE(psb.getObjectId() == 0);
  }

  SECTION("Perform Service") {
    REQUIRE(psb.performServiceCallCnt == 0);
    REQUIRE(psb.performOperation(0) == retval::OK);
    REQUIRE(psb.performServiceCallCnt == 1);
  }

  SECTION("Send Request with Successful Handling") {
    REQUIRE(psb.performServiceCallCnt == 0);
    uint8_t* dataPtr;
    REQUIRE(pool.getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) == retval::OK);
    REQUIRE(creator.serializeBe(dataPtr, creator.getSerializedSize()) == retval::OK);
    tmtcMsg.setStorageId(storeId);
    msgQueue.addReceivedMessage(tmtcMsg);
    REQUIRE(psb.performOperation(0) == retval::OK);
    uint8_t subservice = 0;
    REQUIRE(psb.getAndPopNextSubservice(subservice));
    REQUIRE(subservice == 1);
    REQUIRE(psb.performServiceCallCnt == 1);
    // PSB should take care of freeing the pool slot
    REQUIRE(not pool.hasDataAtId(storeId));
    REQUIRE(verificationReporter.successCallCount() == 1);
    REQUIRE(verificationReporter.failCallCount() == 0);
    auto verifParams = verificationReporter.getNextSuccessCallParams();
    REQUIRE(verifParams.tcPacketId == creator.getPacketIdRaw());
    REQUIRE(verifParams.tcPsc == creator.getPacketSeqCtrlRaw());
  }

  SECTION("Send Request with Failed Handling") {
    uint8_t* dataPtr;
    REQUIRE(pool.getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) == retval::OK);
    REQUIRE(creator.serializeBe(dataPtr, creator.getSerializedSize()) == retval::OK);
    tmtcMsg.setStorageId(storeId);
    msgQueue.addReceivedMessage(tmtcMsg);
    psb.makeNextHandleReqCallFail(3);
    REQUIRE(psb.performOperation(0) == retval::OK);
    uint8_t subservice = 0;
    REQUIRE(psb.getAndPopNextSubservice(subservice));
    REQUIRE(subservice == 1);
    REQUIRE(psb.performServiceCallCnt == 1);
    // PSB should take care of freeing the pool slot
    REQUIRE(not pool.hasDataAtId(storeId));
    REQUIRE(verificationReporter.successCallCount() == 0);
    REQUIRE(verificationReporter.failCallCount() == 1);
    auto verifParams = verificationReporter.getNextFailCallParams();
    REQUIRE(verifParams.tcPacketId == creator.getPacketIdRaw());
    REQUIRE(verifParams.tcPsc == creator.getPacketSeqCtrlRaw());
  }
}
