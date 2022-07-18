#include <fsfw/container/DynamicFIFO.h>
#include <fsfw/container/FIFO.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("Static Fifo Tests", "[TestFifo]") {
  INFO("Fifo Tests");
  struct Test {
    uint64_t number1;
    uint32_t number2;
    uint8_t number3;
    bool operator==(struct Test& other) {
      if ((other.number1 == this->number1) and (other.number1 == this->number1) and
          (other.number1 == this->number1)) {
        return true;
      }
      return false;
    }
  };
  FIFO<Test, 3> fifo;
  std::vector<Test> list;

  struct Test structOne({UINT64_MAX, UINT32_MAX, UINT8_MAX});
  struct Test structTwo({0, 1, 2});
  struct Test structThree({42, 43, 44});
  list.push_back(structThree);
  list.push_back(structTwo);
  list.push_back(structOne);
  SECTION("Insert, retrieval test") {
    REQUIRE(fifo.getMaxCapacity() == 3);
    REQUIRE(fifo.size() == 0);
    REQUIRE(fifo.empty());
    REQUIRE(not fifo.full());

    REQUIRE(fifo.insert(structOne) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.insert(structTwo) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.insert(structThree) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.insert(structTwo) == static_cast<int>(FIFOBase<Test>::FULL));

    struct Test testptr;
    REQUIRE(fifo.peek(&testptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    bool equal = testptr == structOne;
    REQUIRE(equal);
    REQUIRE(fifo.size() == 3);
    REQUIRE(fifo.full());
    REQUIRE(not fifo.empty());

    for (size_t i = 2; i < 3; i--) {
      testptr.number1 = 0;
      testptr.number2 = 0;
      testptr.number3 = 0;
      REQUIRE(fifo.retrieve(&testptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
      equal = testptr == list[i];
      REQUIRE(equal);
      REQUIRE(fifo.size() == i);
    }
    testptr.number1 = 0;
    testptr.number2 = 0;
    testptr.number3 = 0;
    REQUIRE(fifo.retrieve(&testptr) == static_cast<int>(FIFOBase<Test>::EMPTY));
    REQUIRE(fifo.peek(&testptr) == static_cast<int>(FIFOBase<Test>::EMPTY));
    REQUIRE(not fifo.full());
    REQUIRE(fifo.empty());
    REQUIRE(fifo.pop() == static_cast<int>(FIFOBase<Test>::EMPTY));

    REQUIRE(fifo.insert(structOne) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.size() == 1);
    REQUIRE(fifo.insert(structTwo) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.size() == 2);
    REQUIRE(fifo.pop() == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.size() == 1);
    testptr.number1 = 0;
    testptr.number2 = 0;
    testptr.number3 = 0;

    // Test that retrieve and peek will not cause a nullptr dereference
    struct Test* ptr = nullptr;
    REQUIRE(fifo.retrieve(ptr) == static_cast<int>(HasReturnvaluesIF::RETURN_FAILED));
    REQUIRE(fifo.peek(ptr) == static_cast<int>(HasReturnvaluesIF::RETURN_FAILED));

    REQUIRE(fifo.peek(&testptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    equal = testptr == structTwo;
    REQUIRE(equal);
    REQUIRE(fifo.pop() == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.size() == 0);
    REQUIRE(fifo.empty());
  };
  SECTION("Copy Test") {
    REQUIRE(fifo.insert(structOne) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.insert(structTwo) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.insert(structThree) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.size() == 3);
    REQUIRE(fifo.full());
    REQUIRE(not fifo.empty());

    FIFO<Test, 3> fifo2(fifo);
    REQUIRE(fifo2.size() == 3);
    REQUIRE(fifo2.full());
    REQUIRE(not fifo2.empty());
    for (size_t i = 2; i < 3; i--) {
      struct Test testptr = {0, 0, 0};
      REQUIRE(fifo2.retrieve(&testptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
      bool equal = testptr == list[i];
      REQUIRE(equal);
      REQUIRE(fifo2.size() == i);
    }
  };

  SECTION("Assignment Test") {
    REQUIRE(fifo.insert(structOne) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.insert(structTwo) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.insert(structThree) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(fifo.size() == 3);
    REQUIRE(fifo.full());
    REQUIRE(not fifo.empty());

    FIFO<Test, 3> fifo2;
    fifo2 = fifo;
    REQUIRE(fifo2.size() == 3);
    REQUIRE(fifo2.full());
    REQUIRE(not fifo2.empty());
    for (size_t i = 2; i < 3; i--) {
      struct Test testptr = {0, 0, 0};
      REQUIRE(fifo2.retrieve(&testptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
      bool equal = testptr == list[i];
      REQUIRE(equal);
      REQUIRE(fifo2.size() == i);
    }
  };
};
