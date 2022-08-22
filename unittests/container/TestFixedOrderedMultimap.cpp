#include <fsfw/container/FixedOrderedMultimap.h>
#include <fsfw/returnvalues/returnvalue.h>

#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("FixedOrderedMultimap Tests", "[containers]") {
  INFO("FixedOrderedMultimap Tests");

  FixedOrderedMultimap<unsigned int, unsigned short> map(30);
  REQUIRE(map.size() == 0);
  REQUIRE(map.maxSize() == 30);

  SECTION("Test insert, find, exists") {
    for (uint16_t i = 0; i < 30; i++) {
      REQUIRE(map.insert(std::make_pair(i, i + 1)) == static_cast<int>(returnvalue::OK));
      REQUIRE(map.exists(i) == static_cast<int>(returnvalue::OK));
      REQUIRE(map.find(i)->second == i + 1);
    }
    REQUIRE(map.insert(0, 0) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::MAP_FULL));
    REQUIRE(map.exists(31) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.size() == 30);
    {
      uint16_t* ptr;
      REQUIRE(map.find(5, &ptr) == static_cast<int>(returnvalue::OK));
      REQUIRE(*ptr == 6);
      REQUIRE(map.find(31, &ptr) ==
              static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    }
    REQUIRE(map.erase(2) == static_cast<int>(returnvalue::OK));
    REQUIRE(map.erase(31) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.exists(2) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.size() == 29);

    for (auto element : map) {
      if (element.first == 5) {
        REQUIRE(element.second == 6);
      }
    }

    for (FixedOrderedMultimap<uint32_t, uint16_t>::Iterator it = map.begin(); it != map.end();
         it++) {
      REQUIRE(it->second == it->first + 1);
      REQUIRE((*it).second == (*it).first + 1);
      it->second = it->second + 1;
      REQUIRE(it->second == it->first + 2);
    }

    {
      FixedOrderedMultimap<uint32_t, uint16_t>::Iterator it = map.begin();
      while (it != map.end()) {
        REQUIRE(map.erase(&it) == static_cast<int>(returnvalue::OK));
      }
      REQUIRE(map.size() == 0);
    }

    for (FixedOrderedMultimap<uint32_t, uint16_t>::Iterator it = map.begin(); it != map.end();
         it++) {
      // This line should never executed if begin and end is correct
      FAIL("Should never be reached, Iterators invalid");
    }
  };

  SECTION("Test different insert variants") {
    FixedOrderedMultimap<uint32_t, uint16_t>::Iterator it = map.end();
    REQUIRE(map.insert(36, 37, &it) == static_cast<int>(returnvalue::OK));
    REQUIRE(it->first == 36);
    REQUIRE(it->second == 37);
    REQUIRE(map.size() == 1);
    REQUIRE(map.insert(37, 38, nullptr) == static_cast<int>(returnvalue::OK));
    REQUIRE(map.find(37)->second == 38);
    REQUIRE(map.size() == 2);
    REQUIRE(map.insert(37, 24, nullptr) == static_cast<int>(returnvalue::OK));
    REQUIRE(map.find(37)->second == 38);
    REQUIRE(map.insert(0, 1, nullptr) == static_cast<int>(returnvalue::OK));
    REQUIRE(map.find(0)->second == 1);
    REQUIRE(map.size() == 4);
    map.clear();
    REQUIRE(map.size() == 0);
  }
  SECTION("Test different erase and find with no entries") {
    FixedOrderedMultimap<uint32_t, uint16_t>::Iterator it;
    it = map.end();
    REQUIRE(map.erase(&it) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.find(1) == map.end());
  }
}

TEST_CASE("FixedOrderedMultimap Non Trivial Type", "[TestFixedOrderedMultimapNonTrivial]") {
  INFO("FixedOrderedMultimap Non Trivial Type");

  class TestClass {
   public:
    TestClass(){};
    TestClass(uint32_t number1, uint64_t number2) : number1(number1), number2(number2){};
    ~TestClass(){};

    bool operator==(const TestClass& lhs) {
      return ((this->number1 == lhs.number1) and (this->number2 == lhs.number2));
    }
    bool operator!=(const TestClass& lhs) { return not(this->operator==(lhs)); }

    TestClass(const TestClass& other) {
      this->number1 = other.number1;
      this->number2 = other.number2;
    };
    TestClass& operator=(const TestClass& other) {
      this->number1 = other.number1;
      this->number2 = other.number2;
      return *this;
    };

   private:
    uint32_t number1 = 0;
    uint64_t number2 = 5;
  };
  FixedOrderedMultimap<unsigned int, TestClass> map(30);
  REQUIRE(map.size() == 0);
  REQUIRE(map.maxSize() == 30);

  SECTION("Test insert, find, exists") {
    for (uint16_t i = 0; i < 30; i++) {
      REQUIRE(map.insert(std::make_pair(i, TestClass(i + 1, i))) ==
              static_cast<int>(returnvalue::OK));
      REQUIRE(map.exists(i) == static_cast<int>(returnvalue::OK));
      bool compare = map.find(i)->second == TestClass(i + 1, i);
      REQUIRE(compare);
    }
    REQUIRE(map.insert(0, TestClass()) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::MAP_FULL));
    REQUIRE(map.exists(31) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.size() == 30);
    {
      TestClass* ptr = nullptr;
      REQUIRE(map.find(5, &ptr) == static_cast<int>(returnvalue::OK));
      bool compare = *ptr == TestClass(6, 5);
      REQUIRE(compare);
      REQUIRE(map.find(31, &ptr) ==
              static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    }
    REQUIRE(map.erase(2) == static_cast<int>(returnvalue::OK));
    REQUIRE(map.erase(31) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.exists(2) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.size() == 29);

    for (auto element : map) {
      if (element.first == 5) {
        bool compare = element.second == TestClass(6, 5);
        REQUIRE(compare);
      }
    }

    for (FixedOrderedMultimap<uint32_t, TestClass>::Iterator it = map.begin(); it != map.end();
         it++) {
      bool compare = it->second == TestClass(it->first + 1, it->first);
      REQUIRE(compare);
      compare = (*it).second == TestClass((*it).first + 1, (*it).first);
      REQUIRE(compare);
      it->second = TestClass(it->first + 2, it->first);
      compare = it->second == TestClass(it->first + 2, it->first);
      REQUIRE(compare);
    }

    {
      FixedOrderedMultimap<uint32_t, TestClass>::Iterator it = map.begin();
      while (it != map.end()) {
        REQUIRE(map.erase(&it) == static_cast<int>(returnvalue::OK));
      }
      REQUIRE(map.size() == 0);
    }

    for (FixedOrderedMultimap<uint32_t, TestClass>::Iterator it = map.begin(); it != map.end();
         it++) {
      // This line should never executed if begin and end is correct
      FAIL("Should never be reached, Iterators invalid");
    }
  };

  SECTION("Test different insert variants") {
    FixedOrderedMultimap<uint32_t, TestClass>::Iterator it = map.end();
    REQUIRE(map.insert(36, TestClass(37, 36), &it) == static_cast<int>(returnvalue::OK));
    REQUIRE(it->first == 36);
    bool compare = it->second == TestClass(37, 36);
    REQUIRE(compare);
    REQUIRE(map.size() == 1);
    REQUIRE(map.insert(37, TestClass(38, 37), nullptr) == static_cast<int>(returnvalue::OK));
    compare = map.find(37)->second == TestClass(38, 37);
    REQUIRE(compare);
    REQUIRE(map.size() == 2);
    REQUIRE(map.insert(37, TestClass(24, 37), nullptr) == static_cast<int>(returnvalue::OK));
    compare = map.find(37)->second == TestClass(38, 37);
    REQUIRE(compare);
    REQUIRE(map.insert(0, TestClass(1, 0), nullptr) == static_cast<int>(returnvalue::OK));
    compare = map.find(0)->second == TestClass(1, 0);
    REQUIRE(compare);
    REQUIRE(map.size() == 4);
    map.clear();
    REQUIRE(map.size() == 0);
  }
  SECTION("Test different erase and find with no entries") {
    FixedOrderedMultimap<uint32_t, TestClass>::Iterator it;
    it = map.end();
    REQUIRE(map.erase(&it) ==
            static_cast<int>(FixedOrderedMultimap<uint32_t, TestClass>::KEY_DOES_NOT_EXIST));
    REQUIRE(map.find(1) == map.end());
  }
}
