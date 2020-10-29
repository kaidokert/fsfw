#include <fsfw/container/ArrayList.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>
#include <catch2/catch.hpp>
#include "../../core/CatchDefinitions.h"

/**
 * @brief 	Array List test
 */
TEST_CASE("Array List" , "[ArrayListTest]") {
	//perform set-up here
	ArrayList<uint16_t> list(20);
	struct TestClass{
	public:
		TestClass(){};
		TestClass(uint32_t number1,	uint64_t number2):
			number1(number1), number2(number2){};
		uint32_t number1 = -1;
		uint64_t number2 = -1;
		bool operator==(const TestClass& other){
			return ((this->number1 == other.number1) and (this->number2 == other.number2));
		};
	};
	ArrayList<TestClass> complexList(20);
	SECTION("SimpleTest") {
		REQUIRE(list.maxSize()==20);
		REQUIRE(list.size == 0);
		REQUIRE(list.insert(10) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(list[0] == 10);
		REQUIRE(list.front() != nullptr);
		REQUIRE((*list.front()) == 10);
		REQUIRE(list.back() != nullptr);
		REQUIRE((*list.back()) == 10);
		// Need to test the const version of back as well
		const uint16_t* number = const_cast<const ArrayList<uint16_t>*>(&list)->back();
		REQUIRE(*number == 10);
		list.clear();
		REQUIRE(list.size == 0);
	}
	SECTION("Fill and check"){
		//This is an invalid element but its not a nullptr
		REQUIRE(list.back() != nullptr);
		for (auto i =0; i < 20; i++){
			REQUIRE(list.insert(i) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		}
		REQUIRE(list.insert(20) == static_cast<int>(ArrayList<uint16_t>::FULL));
		ArrayList<uint16_t>::Iterator it = list.begin();
		REQUIRE((*it) == 0);
		it++;
		REQUIRE((*it) == 1);
		it--;
		REQUIRE((*it) == 0);
		it++;
		for(auto it2 = list.begin(); it2!=list.end(); it2++){
			if (it == it2){
				REQUIRE((*it) == (*it2));
				break;
			}else{
				REQUIRE((*it2) == 0);
				REQUIRE(it2 != it);
			}
		}
	}
	SECTION("Const Iterator"){
		ArrayList<uint16_t>::Iterator it = list.begin();
		for (auto i =0; i < 10; i++){
			REQUIRE(list.insert(i) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		}
		it++;
		const uint16_t* number = it.value;
		REQUIRE(*number == 1);
	}

	SECTION("Const Iterator"){
		ArrayList<TestClass>::Iterator it = complexList.begin();
		for (auto i =0; i < 10; i++){
			REQUIRE(complexList.insert(TestClass(i, i+1)) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		}
		it++;
		const TestClass* secondTest = it.value;
		bool compare = TestClass(1, 2) == *secondTest;
		REQUIRE(compare);
		it++;
		REQUIRE(it->number1 == 2);
		REQUIRE(it->number2 == 3);
		const ArrayList<TestClass>::Iterator it4(&(complexList[2]));
		REQUIRE(it4->number1 == 2);
		REQUIRE((*it4).number2 == 3);
		REQUIRE(complexList.remaining()==10);
	}
}
