#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/util/dataWrapper.h"
#include "mocks/SimpleSerializable.h"

TEST_CASE("Data Wrapper", "[util]") {
  util::DataWrapper wrapper;
  SECTION("State") {
    REQUIRE(wrapper.isNull());
    REQUIRE(wrapper.type == util::DataTypes::NONE);
  }

  SECTION("Set Raw Data") {
    util::DataWrapper* instance = &wrapper;
    bool deleteInst = false;
    REQUIRE(wrapper.isNull());
    std::array<uint8_t, 4> data = {1, 2, 3, 4};
    SECTION("Setter") {
      wrapper.setRawData({data.data(), data.size()});
    }
    SECTION("Direct Construction Pair") {
      instance = new util::DataWrapper(util::BufPair(data.data(), data.size()));
      deleteInst = true;
    }
    SECTION("Direct Construction Single Args") {
      instance = new util::DataWrapper(data.data(), data.size());
      deleteInst = true;
    }
    REQUIRE(not instance->isNull());
    REQUIRE(instance->type == util::DataTypes::RAW);
    REQUIRE(instance->dataUnion.raw.data == data.data());
    REQUIRE(instance->dataUnion.raw.len == data.size());
    if(deleteInst) {
      delete instance;
    }
  }

  SECTION("Simple Serializable") {
    util::DataWrapper* instance = &wrapper;
    bool deleteInst = false;
    REQUIRE(instance->isNull());
    SimpleSerializable serializable;
    SECTION("Setter") {
      wrapper.setSerializable(serializable);
    }
    SECTION("Direct Construction") {
      instance = new util::DataWrapper(serializable);
      deleteInst = true;
    }

    REQUIRE(not instance->isNull());
    REQUIRE(instance->type == util::DataTypes::SERIALIZABLE);
    REQUIRE(instance->dataUnion.serializable == &serializable);
    if(deleteInst) {
      delete instance;
    }
  }
}