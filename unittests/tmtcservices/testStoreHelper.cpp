#include <catch2/catch_test_macros.hpp>

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"
#include "fsfw/tmtcservices/tmHelpers.h"
#include "mocks/CdsShortTimestamperMock.h"
#include "mocks/SimpleSerializable.h"

TEST_CASE("TM Store Helper", "[tm-store-helper]") {
  auto timeStamper = CdsShortTimestamperMock();
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto storeHelper = TmStoreHelper(2, pool, timeStamper);

  SECTION("State") {
    REQUIRE(storeHelper.getCurrentAddr() == store_address_t::invalid());
    REQUIRE(storeHelper.getTimeStamper() == &timeStamper);
    REQUIRE(storeHelper.getTmStore() == &pool);
    REQUIRE(storeHelper.preparePacket(17, 1, 1) == returnvalue::OK);
    auto& creator = storeHelper.getCreatorRef();
    REQUIRE(creator.getApid() == 2);
    REQUIRE(creator.getService() == 17);
    REQUIRE(creator.getSubService() == 1);
    REQUIRE(creator.getSequenceCount() == 0);
    REQUIRE(creator.getMessageTypeCounter() == 1);
  }

  SECTION("Timestamper Setter") {
    auto timeStamper2 = CdsShortTimestamperMock();
    storeHelper.setTimeStamper(timeStamper2);
    REQUIRE(storeHelper.getTimeStamper() == &timeStamper2);
  }

  SECTION("Pool Setter") {
    LocalPool::LocalPoolConfig cfg2 = {{10, 32}, {5, 64}};
    LocalPool pool2(objects::NO_OBJECT, cfg);
    storeHelper.setTmStore(pool2);
    REQUIRE(storeHelper.getTmStore() == &pool2);
  }

  SECTION("APID Setter") {
    storeHelper.setApid(3);
    auto& creator = storeHelper.getCreatorRef();
    REQUIRE(creator.getApid() == 3);
  }

  SECTION("Basic") {
    REQUIRE(storeHelper.preparePacket(17, 1, 1) == returnvalue::OK);
    REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
    REQUIRE(storeHelper.getCurrentAddr() != store_address_t::invalid());
    auto accessor = pool.getData(storeHelper.getCurrentAddr());
    REQUIRE(accessor.first == returnvalue::OK);
    // Not going to verify individual fields, the creator was unittested separately
    REQUIRE(accessor.second.size() == 22);
  }

  SECTION("Deletion") {
    REQUIRE(storeHelper.preparePacket(17, 1, 1) == returnvalue::OK);
    REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
    REQUIRE(storeHelper.getCurrentAddr() != store_address_t::invalid());
    {
      auto accessor = pool.getData(storeHelper.getCurrentAddr());
      REQUIRE(accessor.first == returnvalue::OK);
      REQUIRE(accessor.second.size() == 22);
      accessor.second.release();
    }
    REQUIRE(storeHelper.deletePacket() == returnvalue::OK);
    REQUIRE(storeHelper.getCurrentAddr() == store_address_t::invalid());
    auto accessor = pool.getData(storeHelper.getCurrentAddr());
    REQUIRE(accessor.first != returnvalue::OK);
  }

  SECTION("With App Data Raw") {
    REQUIRE(storeHelper.preparePacket(17, 1, 1) == returnvalue::OK);
    std::array<uint8_t, 3> data = {1, 2, 3};
    REQUIRE(storeHelper.setSourceDataRaw(data.data(), data.size()) == returnvalue::OK);
    REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
    REQUIRE(storeHelper.getCurrentAddr() != store_address_t::invalid());
    auto accessor = pool.getData(storeHelper.getCurrentAddr());
    REQUIRE(accessor.first == returnvalue::OK);
    // Not going to verify individual fields, the creator was unittested separately
    REQUIRE(accessor.second.size() == 25);
  }

  SECTION("With App Data Serializable") {
    REQUIRE(storeHelper.preparePacket(17, 1, 1) == returnvalue::OK);
    auto serializable = SimpleSerializable();
    REQUIRE(storeHelper.setSourceDataSerializable(serializable) == returnvalue::OK);
    REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
    REQUIRE(storeHelper.getCurrentAddr() != store_address_t::invalid());
    auto accessor = pool.getData(storeHelper.getCurrentAddr());
    REQUIRE(accessor.first == returnvalue::OK);
    // Not going to verify individual fields, the creator was unittested separately
    REQUIRE(accessor.second.size() == 25);
  }

  SECTION("APID Only CTOR") {
    auto storeHelperApidOnly = TmStoreHelper(2);
    REQUIRE(storeHelperApidOnly.getApid() == 2);
    REQUIRE(storeHelperApidOnly.getTmStore() == nullptr);
    REQUIRE(storeHelperApidOnly.getTimeStamper() == nullptr);
  }

  SECTION("APID and TM Store Only CTOR") {
    auto storeHelperApidOnly = TmStoreHelper(2, pool);
    REQUIRE(storeHelperApidOnly.getApid() == 2);
    REQUIRE(storeHelperApidOnly.getTmStore() == &pool);
    REQUIRE(storeHelperApidOnly.getTimeStamper() == nullptr);
  }
}