#include <fsfw/container/ArrayList.h>
#include <fsfw/container/PlacementFactory.h>
#include <fsfw/returnvalues/returnvalue.h>
#include <fsfw/storagemanager/LocalPool.h>

#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("PlacementFactory Tests", "[containers]") {
  INFO("PlacementFactory Tests");

  LocalPool::LocalPoolConfig poolCfg = {
      {1, sizeof(uint16_t)}, {1, sizeof(uint32_t)}, {1, sizeof(uint64_t)}};
  // const uint16_t element_sizes[3] = {sizeof(uint16_t), sizeof(uint32_t), sizeof(uint64_t)};
  // const uint16_t n_elements[3] = {1, 1, 1};
  LocalPool storagePool(0x1, poolCfg, false, true);
  PlacementFactory factory(&storagePool);

  SECTION("Pool overload") {
    store_address_t address;
    uint8_t* ptr = nullptr;
    REQUIRE(storagePool.getFreeElement(&address, sizeof(ArrayList<uint32_t, uint16_t>), &ptr) ==
            static_cast<int>(StorageManagerIF::DATA_TOO_LARGE));
    ArrayList<uint32_t, uint16_t>* list2 = factory.generate<ArrayList<uint32_t, uint16_t> >(80);
    REQUIRE(list2 == nullptr);
  }

  SECTION("Test generate and destroy") {
    uint64_t* number = factory.generate<uint64_t>(32000);
    REQUIRE(number != nullptr);
    REQUIRE(*number == 32000);
    store_address_t address;
    uint8_t* ptr = nullptr;
    REQUIRE(storagePool.getFreeElement(&address, sizeof(uint64_t), &ptr) ==
            static_cast<int>(StorageManagerIF::DATA_TOO_LARGE));
    uint64_t* number2 = factory.generate<uint64_t>(12345);
    REQUIRE(number2 == nullptr);
    REQUIRE(factory.destroy(number) == static_cast<int>(returnvalue::OK));
    REQUIRE(storagePool.getFreeElement(&address, sizeof(uint64_t), &ptr) ==
            static_cast<int>(returnvalue::OK));
    REQUIRE(storagePool.deleteData(address) == static_cast<int>(returnvalue::OK));

    // Check that PlacementFactory checks for nullptr
    ptr = nullptr;
    REQUIRE(factory.destroy(ptr) == static_cast<int>(returnvalue::FAILED));
  }
}
