#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/util/dataWrapper.h"
#include "mocks/SimpleSerializable.h"

TEST_CASE("Data Wrapper", "[util]") {
  util::DataWrapper wrapper;
  SECTION("State") {
    REQUIRE(wrapper.isNull());
  }

  SECTION("Set Raw Data") {
    REQUIRE(wrapper.isNull());
    std::array<uint8_t, 4> data = {1, 2, 3, 4};
    wrapper.setRawData({data.data(), data.size()});
    REQUIRE(not wrapper.isNull());
    REQUIRE(wrapper.type == util::DataTypes::RAW);
    REQUIRE(wrapper.dataUnion.raw.data == data.data());
    REQUIRE(wrapper.dataUnion.raw.len == data.size());
  }

  SECTION("Simple Serializable") {
    REQUIRE(wrapper.isNull());
    SimpleSerializable serializable;
    wrapper.setSerializable(serializable);
    REQUIRE(not wrapper.isNull());
    REQUIRE(wrapper.type == util::DataTypes::SERIALIZABLE);
    REQUIRE(wrapper.dataUnion.serializable == &serializable);
  }
}