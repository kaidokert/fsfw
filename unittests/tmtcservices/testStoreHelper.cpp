#include <catch2/catch_test_macros.hpp>

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"

TEST_CASE("TM Store Helper", "[tm-store-helper]") {
  LocalPool::LocalPoolConfig cfg = {{10, 32}, {5, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto storeHelper = TmStoreHelper(2, &pool);
}