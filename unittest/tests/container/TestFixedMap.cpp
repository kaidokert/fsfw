#include <fsfw/container/FixedMap.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#include <catch2/catch.hpp>
#include "../../core/CatchDefinitions.h"

template class FixedMap<unsigned int, unsigned short>;

TEST_CASE( "FixedMap Tests", "[TestFixedMap]") {
	INFO("FixedMap Tests");

	FixedMap<unsigned int, unsigned short> map(30);
	REQUIRE(map.size() == 0);
	REQUIRE(map.maxSize() == 30);
	REQUIRE(map.getSerializedSize() == sizeof(uint32_t));
	REQUIRE(map.empty());
	REQUIRE(not map.full());

	SECTION("Fill and erase"){
		for (uint16_t i=0;i<30;i++){
			REQUIRE(map.insert(std::make_pair(i, i+1))== static_cast<int>(HasReturnvaluesIF::RETURN_OK));
			REQUIRE(map.exists(i) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
			REQUIRE(map.find(i)->second==i+1);
			REQUIRE(not map.empty());
		}
		REQUIRE(map.insert(0, 0) == static_cast<int>(FixedMap<uint32_t, uint16_t>::KEY_ALREADY_EXISTS));
		REQUIRE(map.insert(31, 0) == static_cast<int>(FixedMap<uint32_t, uint16_t>::MAP_FULL));
		REQUIRE(map.exists(31) == static_cast<int>(FixedMap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
		REQUIRE(map.size() == 30);
		REQUIRE(map.full());
		{
			uint16_t* ptr;
			REQUIRE(map.find(5,&ptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
			REQUIRE(*ptr == 6);
			REQUIRE(*(map.findValue(6)) == 7);
			REQUIRE(map.find(31,&ptr) == static_cast<int>(FixedMap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
		}

		REQUIRE(map.getSerializedSize() == (sizeof(uint32_t)+ 30*(sizeof(uint32_t) + sizeof(uint16_t))));
		REQUIRE(map.erase(2) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(map.erase(31) == static_cast<int>(FixedMap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
		REQUIRE(map.exists(2) == static_cast<int>(FixedMap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
		REQUIRE(map.size() == 29);

		for (auto element: map){
			if (element.first == 5){
				REQUIRE(element.second == 6);
			}
		}

		for (FixedMap<uint32_t, uint16_t>::Iterator it = map.begin(); it != map.end(); it++){
			REQUIRE(it->second == it->first + 1);
			REQUIRE((*it).second == (*it).first + 1);
			it->second = it->second + 1;
			REQUIRE(it->second == it->first + 2);
		}

		for (FixedMap<uint32_t, uint16_t>::Iterator it = map.begin(); it != map.end(); it++){
			REQUIRE(map.erase(&it) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		}

		REQUIRE(map.size() == 0);

		for (FixedMap<uint32_t, uint16_t>::Iterator it = map.begin(); it != map.end(); it++){
			// This line should never executed if begin and end is correct
			FAIL("Should never be reached, Iterators invalid");
		}
	};

	SECTION("Insert variants"){
		FixedMap<uint32_t, uint16_t>::Iterator it = map.end();
		REQUIRE(map.insert(36, 37, &it) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(it->first == 36);
		REQUIRE(it->second == 37);
		REQUIRE(map.size() == 1);
		REQUIRE(map.insert(37, 38, nullptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(map.find(37)->second == 38);
		REQUIRE(map.size() == 2);
		REQUIRE(map.insert(37, 24, nullptr) == static_cast<int>(FixedMap<uint32_t, uint16_t>::KEY_ALREADY_EXISTS));
		REQUIRE(map.find(37)->second != 24);
		REQUIRE(map.size() == 2);
	};
	SECTION("Serialize and DeSerialize") {
		REQUIRE(map.insert(36, 37, nullptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(map.insert(37, 38, nullptr) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		uint8_t buffer[sizeof(uint32_t)
				+ 2 * (sizeof(uint32_t) + sizeof(uint16_t))];
		REQUIRE(
				map.getSerializedSize()
						== (sizeof(uint32_t)
								+ 2 * (sizeof(uint32_t) + sizeof(uint16_t))));
		uint8_t *loc_ptr = buffer;
		size_t size = 0;
		REQUIRE(
				map.serialize(&loc_ptr, &size, 10, SerializeIF::Endianness::BIG)
						== static_cast<int>(SerializeIF::BUFFER_TOO_SHORT));
		loc_ptr = buffer;
		size = 0;
		REQUIRE(
				map.serialize(&loc_ptr, &size,
						sizeof(uint32_t)
								+ 2 * (sizeof(uint32_t) + sizeof(uint16_t)),
						SerializeIF::Endianness::BIG)
						== static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(size == 16);

		uint32_t internal_size = 0;
		const uint8_t *ptr2 = buffer;
		REQUIRE(
				SerializeAdapter::deSerialize(&internal_size, &ptr2, &size,
						SerializeIF::Endianness::BIG)
						== static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(internal_size == 2);
		for (uint8_t i = 36; i < 38; i++) {
			uint32_t first_element = 0;
			REQUIRE(
					SerializeAdapter::deSerialize(&first_element, &ptr2, &size,
							SerializeIF::Endianness::BIG)
							== static_cast<int>(HasReturnvaluesIF::RETURN_OK));
			REQUIRE(first_element == i);
			uint16_t second_element = 0;
			REQUIRE(
					SerializeAdapter::deSerialize(&second_element, &ptr2, &size,
							SerializeIF::Endianness::BIG)
							== static_cast<int>(HasReturnvaluesIF::RETURN_OK));
			REQUIRE(second_element == i + 1);
		}
		REQUIRE(size == 0);
		map.clear();
		const uint8_t* constPtr = buffer;
		size = 16;
		REQUIRE(map.size() == 0);
		REQUIRE(map.deSerialize(&constPtr, &size,
				SerializeIF::Endianness::BIG) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(map.size() == 2);
		REQUIRE(map.find(36)->second == 37);
		for(auto& element: map){
			REQUIRE((element.first+1) == element.second);
		}
	};


	SECTION("Failed erase and deSerialize"){
		FixedMap<uint32_t, uint16_t>::Iterator it;
		std::pair<uint32_t, uint16_t> pair = std::make_pair(44, 43);
		it = FixedMap<uint32_t, uint16_t>::Iterator(&pair);
		REQUIRE(map.erase(&it) == static_cast<int>(FixedMap<uint32_t, uint16_t>::KEY_DOES_NOT_EXIST));
		REQUIRE(map.find(45) == map.end());
		size_t toLargeMap = 100;
		const uint8_t* ptr = reinterpret_cast<uint8_t*>(&toLargeMap);
		size_t size = sizeof(size_t);
		REQUIRE(map.deSerialize(&ptr, &size, SerializeIF::Endianness::BIG) ==
				static_cast<int>(SerializeIF::TOO_MANY_ELEMENTS));
	};
	SECTION("Little Endianess"){
		map.clear();
		map.insert(10,20, nullptr);
		uint8_t newBuffer[sizeof(uint32_t)+ 1*(sizeof(uint32_t) + sizeof(uint16_t))];
		uint8_t* ptr = newBuffer;
		size_t size =  0;
		size_t max_size = sizeof(uint32_t)+ 1*(sizeof(uint32_t) + sizeof(uint16_t));
		REQUIRE(map.serialize(&ptr, &size, max_size,
				SerializeIF::Endianness::LITTLE) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		map.clear();
		REQUIRE(map.size()==0);
		const uint8_t* ptr2 = newBuffer;
		REQUIRE(map.deSerialize(&ptr2, &size,
				SerializeIF::Endianness::LITTLE) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		REQUIRE(map.size()==1);
		REQUIRE(map.find(10)->second == 20);
	};
}
