#include <catch2/catch_test_macros.hpp>

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"
#include "mocks/CdsShortTimestamperMock.h"

TEST_CASE("TM Store Helper", "[tm-store-helper]") {
  auto timeStamper = CdsShortTimestamperMock();
  LocalPool::LocalPoolConfig cfg = {{10, 32}, {5, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto storeHelper = TmStoreHelper(2, pool, timeStamper);

  SECTION("State") {
    REQUIRE(storeHelper.getCurrentAddr() == store_address_t::invalid());
    REQUIRE(storeHelper.preparePacket(17, 1, 1) == HasReturnvaluesIF::RETURN_OK);
    auto& creator = storeHelper.getCreatorRef();
    REQUIRE(creator.getApid() == 2);
    REQUIRE(creator.getService() == 17);
    REQUIRE(creator.getSubService() == 1);
    REQUIRE(creator.getSequenceCount() == 0);
    REQUIRE(creator.getMessageTypeCounter() == 1);
  }

  SECTION("Basic") {
    REQUIRE(storeHelper.preparePacket(17, 1, 1) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(storeHelper.addPacketToStore() == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(storeHelper.getCurrentAddr() != store_address_t::invalid());
  }
}