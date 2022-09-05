#include <catch2/catch_test_macros.hpp>

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcservices/TmSendHelper.h"
#include "fsfw/tmtcservices/TmStoreAndSendHelper.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"
#include "mocks/CdsShortTimestamperMock.h"
#include "mocks/InternalErrorReporterMock.h"
#include "mocks/MessageQueueMock.h"
#include "mocks/SimpleSerializable.h"

TEST_CASE("TM Store And Send Helper", "[tm-store-send-helper]") {
  auto timeStamper = CdsShortTimestamperMock();
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto storeHelper = TmStoreHelper(2, pool, timeStamper);

  MessageQueueId_t destId = 1;
  auto errReporter = InternalErrorReporterMock();
  auto msgQueue = MessageQueueMock(2);
  msgQueue.setDefaultDestination(destId);
  TmSendHelper sendHelper(msgQueue, errReporter, destId);
  TmStoreAndSendWrapper tmHelper(17, storeHelper, sendHelper);

  SECTION("State") {
    CHECK(tmHelper.sendCounter == 0);
    CHECK(tmHelper.defaultService == 17);
    CHECK(tmHelper.delOnFailure);
    CHECK(tmHelper.incrementSendCounter);
    CHECK(&tmHelper.sendHelper == &sendHelper);
    CHECK(&tmHelper.storeHelper == &storeHelper);
  }

  SECTION("Storage Fails") {
    // Too large to fit in store
    std::array<uint8_t, 80> data{};
    REQUIRE(storeHelper.setSourceDataRaw(data.data(), data.size()) == returnvalue::OK);
    REQUIRE(tmHelper.storeAndSendTmPacket() == StorageManagerIF::DATA_TOO_LARGE);
  }

  SECTION("Base Test") {
    tmHelper.prepareTmPacket(2);
    auto& creator = storeHelper.getCreatorRef();
    REQUIRE(creator.getSubService() == 2);
    REQUIRE(creator.getService() == 17);
    auto& params = creator.getParams();
    REQUIRE(params.sourceData == nullptr);
    REQUIRE(tmHelper.sendCounter == 0);
    REQUIRE(tmHelper.storeAndSendTmPacket() == returnvalue::OK);
    REQUIRE(tmHelper.sendCounter == 1);
    auto storeId = storeHelper.getCurrentAddr();
    REQUIRE(msgQueue.wasMessageSent());
    REQUIRE(msgQueue.numberOfSentMessagesToDefault() == 1);
    TmTcMessage msg;
    REQUIRE(msgQueue.getNextSentMessage(msg) == returnvalue::OK);
    REQUIRE(msg.getStorageId() == storeId);
    REQUIRE(pool.hasDataAtId(msg.getStorageId()));
    storeHelper.deletePacket();
  }

  SECTION("Raw Data Helper") {
    std::array<uint8_t, 3> data = {1, 2, 3};
    REQUIRE(tmHelper.prepareTmPacket(2, data.data(), data.size()) == returnvalue::OK);
    auto& creator = storeHelper.getCreatorRef();
    auto& params = creator.getParams();
    REQUIRE(params.sourceData != nullptr);
    REQUIRE(params.sourceData->getSerializedSize() == data.size());
    REQUIRE(params.adapter.getConstBuffer() == data.data());
  }

  SECTION("Serializable Helper") {
    auto simpleSer = SimpleSerializable();
    REQUIRE(tmHelper.prepareTmPacket(2, simpleSer) == returnvalue::OK);
    auto& creator = storeHelper.getCreatorRef();
    auto& params = creator.getParams();
    REQUIRE(params.sourceData == &simpleSer);
  }

  SECTION("Object ID prefix Helper") {
    uint32_t objectId = 0x01020304;
    std::array<uint8_t, 3> data = {1, 2, 3};
    telemetry::DataWithObjectIdPrefix dataWithObjId(objectId, data.data(), data.size());
    REQUIRE(tmHelper.prepareTmPacket(2, dataWithObjId) == returnvalue::OK);
    auto& creator = storeHelper.getCreatorRef();
    auto& params = creator.getParams();
    REQUIRE(params.sourceData == &dataWithObjId);
  }

  // TODO: Error handling
}
