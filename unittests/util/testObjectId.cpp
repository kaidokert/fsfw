#include <catch2/catch_test_macros.hpp>

#include "fsfw/util/ObjectId.h"
#include <array>

TEST_CASE("Object Id", "[object-id]") {
    auto objectId = ObjectId(10, "TEST_ID");
    std::map<ObjectId, int> testMap;

    SECTION("State") {
      CHECK(objectId.id() == 10);
      CHECK(std::strcmp(objectId.name(), "TEST_ID") == 0);
    }

    SECTION("ID as map key") {
      auto insertPair = testMap.emplace(objectId, 10);
      CHECK(insertPair.second);
      auto iter = testMap.find(objectId);
      CHECK(iter != testMap.end());
      CHECK(std::strcmp(iter->first.name(), "TEST_ID") == 0);
      CHECK(iter->second == 10);
      auto otherIdSameName = ObjectId(12, "TEST_ID");
      insertPair = testMap.emplace(otherIdSameName, 10);
      CHECK(insertPair.second);
    }
}