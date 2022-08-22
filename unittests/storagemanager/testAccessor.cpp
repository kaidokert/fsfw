#include <fsfw/storagemanager/LocalPool.h>

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstring>

#include "CatchDefinitions.h"

TEST_CASE("Pool Accessor", "[pool-accessor]") {
  LocalPool::LocalPoolConfig poolCfg = {{1, 10}};
  LocalPool simplePool = LocalPool(0, poolCfg);
  std::array<uint8_t, 20> testDataArray{};
  std::array<uint8_t, 20> receptionArray{};
  store_address_t testStoreId;
  ReturnValue_t result = returnvalue::FAILED;

  for (size_t i = 0; i < testDataArray.size(); i++) {
    testDataArray[i] = i;
  }
  size_t size = 10;

  SECTION("Simple tests getter functions") {
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    auto resultPair = simplePool.getData(testStoreId);
    REQUIRE(resultPair.first == returnvalue::OK);
    resultPair.second.getDataCopy(receptionArray.data(), 20);
    CHECK(resultPair.second.getId() == testStoreId);
    CHECK(resultPair.second.size() == 10);
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }

    std::copy(resultPair.second.data(), resultPair.second.data() + resultPair.second.size(),
              receptionArray.data());
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }

    {
      auto resultPairLoc = simplePool.getData(testStoreId);
      REQUIRE(resultPairLoc.first == returnvalue::OK);
      // data should be deleted when accessor goes out of scope.
    }
    resultPair = simplePool.getData(testStoreId);
    REQUIRE(resultPair.first == (int)StorageManagerIF::DATA_DOES_NOT_EXIST);

    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    {
      ConstStorageAccessor constAccessor(testStoreId);
      result = simplePool.getData(testStoreId, constAccessor);
      REQUIRE(result == returnvalue::OK);
      constAccessor.getDataCopy(receptionArray.data(), 20);
      for (size_t i = 0; i < size; i++) {
        CHECK(receptionArray[i] == i);
      }
      // likewise, data should be deleted when accessor gets out of scope.
    }
    resultPair = simplePool.getData(testStoreId);
    REQUIRE(resultPair.first == (int)StorageManagerIF::DATA_DOES_NOT_EXIST);

    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    {
      resultPair = simplePool.getData(testStoreId);
      REQUIRE(resultPair.first == returnvalue::OK);
      resultPair.second.release();
      // now data should not be deleted anymore
    }
    resultPair = simplePool.getData(testStoreId);
    REQUIRE(resultPair.first == returnvalue::OK);
    resultPair.second.getDataCopy(receptionArray.data(), 20);
    for (size_t i = 0; i < size; i++) {
      CHECK(receptionArray[i] == i);
    }
  }

  SECTION("Simple tests modify functions") {
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    {
      StorageAccessor accessor(testStoreId);
      result = simplePool.modifyData(testStoreId, accessor);
      REQUIRE(result == returnvalue::OK);
      CHECK(accessor.getId() == testStoreId);
      CHECK(accessor.size() == 10);
      accessor.getDataCopy(receptionArray.data(), 20);
      for (size_t i = 0; i < size; i++) {
        CHECK(receptionArray[i] == i);
      }
      std::copy(accessor.data(), accessor.data() + accessor.size(), receptionArray.data());
      for (size_t i = 0; i < size; i++) {
        CHECK(receptionArray[i] == i);
      }
      // data should be deleted when accessor goes out of scope
    }
    auto resultPair = simplePool.getData(testStoreId);
    REQUIRE(resultPair.first == (int)StorageManagerIF::DATA_DOES_NOT_EXIST);

    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    {
      auto resultPairLoc = simplePool.modifyData(testStoreId);
      REQUIRE(resultPairLoc.first == returnvalue::OK);
      CHECK(resultPairLoc.second.getId() == testStoreId);
      CHECK(resultPairLoc.second.size() == 10);
      resultPairLoc.second.getDataCopy(receptionArray.data(), 20);
      for (size_t i = 0; i < size; i++) {
        CHECK(receptionArray[i] == i);
      }
      std::copy(resultPairLoc.second.data(),
                resultPairLoc.second.data() + resultPairLoc.second.size(), receptionArray.data());
      for (size_t i = 0; i < size; i++) {
        CHECK(receptionArray[i] == i);
      }
      resultPairLoc.second.release();
      // data should not be deleted when accessor goes out of scope
    }
    resultPair = simplePool.getData(testStoreId);
    REQUIRE(resultPair.first == returnvalue::OK);
  }

  SECTION("Write tests") {
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    {
      auto resultPair = simplePool.modifyData(testStoreId);
      REQUIRE(resultPair.first == returnvalue::OK);
      testDataArray[9] = 42;
      resultPair.second.write(testDataArray.data(), 10, 0);
      // now data should not be deleted
      resultPair.second.release();
    }
    auto resultConstPair = simplePool.getData(testStoreId);
    REQUIRE(resultConstPair.first == returnvalue::OK);

    resultConstPair.second.getDataCopy(receptionArray.data(), 10);
    for (size_t i = 0; i < size - 1; i++) {
      CHECK(receptionArray[i] == i);
    }
    CHECK(receptionArray[9] == 42);

    auto resultPair = simplePool.modifyData(testStoreId);
    REQUIRE(resultPair.first == returnvalue::OK);
    result = resultPair.second.write(testDataArray.data(), 20, 0);
    REQUIRE(result == returnvalue::FAILED);
    result = resultPair.second.write(testDataArray.data(), 10, 5);
    REQUIRE(result == returnvalue::FAILED);

    std::memset(testDataArray.data(), 42, 5);
    result = resultPair.second.write(testDataArray.data(), 5, 5);
    REQUIRE(result == returnvalue::OK);
    resultConstPair = simplePool.getData(testStoreId);
    resultPair.second.getDataCopy(receptionArray.data(), 20);
    for (size_t i = 5; i < 10; i++) {
      CHECK(receptionArray[i] == 42);
    }
  }

  SECTION("Operators") {
    result = simplePool.addData(&testStoreId, testDataArray.data(), size);
    REQUIRE(result == returnvalue::OK);
    {
      StorageAccessor accessor(testStoreId);
      StorageAccessor accessor2(store_address_t::invalid());
      accessor2 = std::move(accessor);
      REQUIRE(accessor.data() == nullptr);
      std::array<uint8_t, 6> data;
      size_t size = 6;
      result = accessor.write(data.data(), data.size());
      REQUIRE(result == returnvalue::FAILED);
      result = simplePool.modifyData(testStoreId, accessor2);
      REQUIRE(result == returnvalue::OK);
      CHECK(accessor2.getId() == testStoreId);
      CHECK(accessor2.size() == 10);

      std::array<uint8_t, 10> newData;
      // Expect data to be invalid so this must return returnvalue::FAILED
      result = accessor.getDataCopy(newData.data(), newData.size());
      REQUIRE(result == returnvalue::FAILED);
      // Expect data to be too small
      result = accessor2.getDataCopy(data.data(), data.size());
      REQUIRE(result == returnvalue::FAILED);
    }
  }
}
