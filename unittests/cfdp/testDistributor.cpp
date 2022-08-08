#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/CfdpDistributor.h"
#include "fsfw/cfdp/pdu/MetadataPduCreator.h"
#include "fsfw/storagemanager/LocalPool.h"
#include "mocks/AcceptsTcMock.h"
#include "mocks/MessageQueueMock.h"

TEST_CASE("CFDP Distributor", "[cfdp][distributor]") {
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto queue = MessageQueueMock(1);
  CfdpDistribCfg distribCfg(1, pool, &queue);
  auto distributor = CfdpDistributor(distribCfg);
  auto entityId = cfdp::EntityId(UnsignedByteField<uint16_t>(2));
  MessageQueueId_t acceptorQueueId = 3;
  auto tcAcceptor = AcceptsTcMock("TC Acceptor", 0, acceptorQueueId);
  SECTION("State") {
    CHECK(distributor.initialize() == result::OK);
    CHECK(std::strcmp(distributor.getName(), "CFDP Distributor") == 0);
    CHECK(distributor.getIdentifier() == 0);
    CHECK(distributor.getRequestQueue() == queue.getId());
  }

  SECTION("Register Listener") {
    CHECK(distributor.initialize() == result::OK);
    CHECK(distributor.registerTcDestination(entityId, tcAcceptor));
    // queue.addReceivedMessage()
  }
}