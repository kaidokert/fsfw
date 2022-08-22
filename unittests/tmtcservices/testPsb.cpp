#include <catch2/catch_test_macros.hpp>

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/storagemanager/PoolManager.h"
#include "mocks/AcceptsTmMock.h"
#include "mocks/CdsShortTimestamperMock.h"
#include "mocks/InternalErrorReporterMock.h"
#include "mocks/MessageQueueMock.h"
#include "mocks/PusDistributorMock.h"
#include "mocks/PusServiceBaseMock.h"
#include "mocks/PusVerificationReporterMock.h"

TEST_CASE("Pus Service Base", "[pus-service-base]") {
  uint16_t apid = 2;
  auto verificationReporter = PusVerificationReporterMock();
  auto msgQueue = MessageQueueMock(1);
  auto tmReceiver = AcceptsTmMock(2);
  auto psbParams = PsbParams(0, apid, 17);

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

  REQUIRE(psb.initialize() == returnvalue::OK);

  SECTION("State") {
    REQUIRE(psb.getIdentifier() == 17);
    REQUIRE(psb.getObjectId() == 0);
    REQUIRE(psb.getRequestQueue() == msgQueue.getId());
    auto psbParamsLocal = psb.getParams();
    REQUIRE(psbParamsLocal.errReporter == nullptr);
  }

  SECTION("Perform Service") {
    REQUIRE(psb.performServiceCallCnt == 0);
    REQUIRE(psb.performOperation(0) == returnvalue::OK);
    REQUIRE(psb.performServiceCallCnt == 1);
  }

  SECTION("Send Request with Successful Handling") {
    REQUIRE(psb.performServiceCallCnt == 0);
    uint8_t* dataPtr;
    REQUIRE(pool.getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) ==
            returnvalue::OK);
    size_t serLen = 0;
    REQUIRE(creator.serializeBe(dataPtr, serLen, creator.getSerializedSize()) == returnvalue::OK);
    tmtcMsg.setStorageId(storeId);
    msgQueue.addReceivedMessage(tmtcMsg);
    REQUIRE(psb.performOperation(0) == returnvalue::OK);
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
    REQUIRE(pool.getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) ==
            returnvalue::OK);
    size_t serLen = 0;
    REQUIRE(creator.serializeBe(dataPtr, serLen, creator.getSerializedSize()) == returnvalue::OK);
    tmtcMsg.setStorageId(storeId);
    msgQueue.addReceivedMessage(tmtcMsg);
    psb.makeNextHandleReqCallFail(3);
    REQUIRE(psb.performOperation(0) == returnvalue::OK);
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

  SECTION("Invalid Packet Sent") {
    tmtcMsg.setStorageId(store_address_t::invalid());
    msgQueue.addReceivedMessage(tmtcMsg);
    REQUIRE(psb.performOperation(0) == returnvalue::OK);
    REQUIRE(verificationReporter.failCallCount() == 1);
    auto verifParams = verificationReporter.getNextFailCallParams();
    REQUIRE(verifParams.tcPacketId == 0);
    REQUIRE(verifParams.tcPsc == 0);
  }

  SECTION("Set Verif Reporter") {
    auto verificationReporter2 = PusVerificationReporterMock();
    psb.setVerificationReporter(verificationReporter2);
    auto& p = psb.getParams();
    REQUIRE(p.verifReporter == &verificationReporter2);
  }

  SECTION("Set Request Queue") {
    auto msgQueueMock = MessageQueueMock(2);
    psb.setRequestQueue(msgQueueMock);
    auto& p = psb.getParams();
    REQUIRE(p.reqQueue == &msgQueueMock);
  }

  SECTION("Set TM Receiver") {
    auto tmReceiver2 = AcceptsTmMock(3);
    psb.setTmReceiver(tmReceiver2);
    auto& p = psb.getParams();
    REQUIRE(p.tmReceiver == &tmReceiver2);
  }

  SECTION("Set TC Store") {
    LocalPool tcStore2(5, cfg);
    psb.setTcPool(tcStore2);
    auto& p = psb.getParams();
    REQUIRE(p.tcPool == &tcStore2);
  }

  SECTION("Set error reporter") {
    auto errReporter = InternalErrorReporterMock();
    psb.setErrorReporter(errReporter);
    auto& p = psb.getParams();
    REQUIRE(p.errReporter == &errReporter);
  }
  SECTION("Owner Queue") {
    // This will cause the initialize function to create a new owner queue
    psbParams.reqQueue = nullptr;
    psbParams.objectId = 1;
    auto mockWithOwnerQueue = PsbMock(psbParams);
    REQUIRE(mockWithOwnerQueue.getRequestQueue() == MessageQueueIF::NO_QUEUE);
    REQUIRE(mockWithOwnerQueue.initialize() == returnvalue::OK);
    REQUIRE(mockWithOwnerQueue.getRequestQueue() != MessageQueueIF::NO_QUEUE);
  }

  SECTION("TM Store Helper Initializer") {
    TmStoreHelper storeHelper(0);
    psb.initializeTmStoreHelper(storeHelper);
    REQUIRE(storeHelper.getApid() == apid);
  }

  SECTION("TM Send Helper Initializer") {
    TmSendHelper sendHelper;
    psb.initializeTmSendHelper(sendHelper);
    REQUIRE(sendHelper.getMsgQueue() == &msgQueue);
    REQUIRE(sendHelper.getDefaultDestination() == msgQueue.getDefaultDestination());
  }

  SECTION("TM Store And Send Helper Initializer") {
    TmStoreHelper storeHelper(0);
    TmSendHelper sendHelper;
    psb.initializeTmHelpers(sendHelper, storeHelper);
    REQUIRE(sendHelper.getMsgQueue() == &msgQueue);
    REQUIRE(sendHelper.getDefaultDestination() == msgQueue.getDefaultDestination());
    REQUIRE(storeHelper.getApid() == apid);
  }

  SECTION("TM Send Helper Initializer With Error Reporter") {
    TmSendHelper sendHelper;
    auto errReporter = InternalErrorReporterMock();
    psb.setErrorReporter(errReporter);
    psb.initializeTmSendHelper(sendHelper);
    REQUIRE(sendHelper.getMsgQueue() == &msgQueue);
    REQUIRE(sendHelper.getDefaultDestination() == msgQueue.getDefaultDestination());
    REQUIRE(sendHelper.getInternalErrorReporter() == &errReporter);
  }

  SECTION("Auto Initialize PUS Distributor") {
    psbParams.objectId = 1;
    object_id_t distributorId = 3;
    auto psb2 = PsbMock(psbParams);
    auto pusDistrib = PusDistributorMock(distributorId);
    PsbMock::setStaticPusDistributor(distributorId);
    REQUIRE(PsbMock::getStaticPusDistributor() == distributorId);
    REQUIRE(psb2.initialize() == returnvalue::OK);
    REQUIRE(pusDistrib.registerCallCount == 1);
    REQUIRE(pusDistrib.registeredServies.front() == &psb2);
  }

  SECTION("Auto Initialize Packet Destination") {
    psbParams.tmReceiver = nullptr;
    psbParams.objectId = 1;
    object_id_t destId = 3;
    auto psb2 = PsbMock(psbParams);
    auto packetDest = AcceptsTmMock(destId, 2);
    PsbMock::setStaticTmDest(destId);
    REQUIRE(PsbMock::getStaticTmDest() == destId);
    REQUIRE(psb2.initialize() == returnvalue::OK);
    auto& p = psb2.getParams();
    REQUIRE(p.tmReceiver == &packetDest);
  }

  SECTION("Auto Initialize Verification Reporter") {
    psbParams.verifReporter = nullptr;
    psbParams.objectId = 1;
    object_id_t reporterId = objects::VERIFICATION_REPORTER;
    PusVerificationReporterMock otherReporter(reporterId);
    auto psb2 = PsbMock(psbParams);
    REQUIRE(psb2.initialize() == returnvalue::OK);
    auto& p = psb2.getParams();
    REQUIRE(p.verifReporter == &otherReporter);
  }

  SECTION("Auto Initialize TC Pool") {
    PoolManager tcStoreGlobal(objects::TC_STORE, cfg);
    psbParams.tcPool = nullptr;
    psbParams.objectId = 1;
    auto psb2 = PsbMock(psbParams);
    REQUIRE(psb2.initialize() == returnvalue::OK);
    auto& p = psb2.getParams();
    REQUIRE(p.tcPool == &tcStoreGlobal);
  }

  SECTION("Invalid Verification Reporter") {
    psbParams.verifReporter = nullptr;
    psbParams.objectId = 1;
    auto psb2 = PsbMock(psbParams);
    REQUIRE(psb2.initialize() == ObjectManagerIF::CHILD_INIT_FAILED);
  }

  SECTION("Invalid TC Store") {
    psbParams.tcPool = nullptr;
    psbParams.objectId = 1;
    auto psb2 = PsbMock(psbParams);
    REQUIRE(psb2.initialize() == ObjectManagerIF::CHILD_INIT_FAILED);
  }
}
