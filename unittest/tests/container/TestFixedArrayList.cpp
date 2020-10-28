#include "../../core/CatchDefinitions.h"

#include <fsfw/container/FixedArrayList.h>
#include <fsfw/returnvalues/HasReturnvaluesIF.h>

#include <catch2/catch.hpp>


TEST_CASE( "FixedArrayList Tests", "[TestFixedArrayList]") {
	INFO("FixedArrayList Tests");
	using testList = FixedArrayList<uint32_t, 260, uint16_t>;
	testList list;
	REQUIRE(list.size==0);
	REQUIRE(list.insert(10) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
	REQUIRE(list.size==1);
	REQUIRE(list.maxSize()==260);
	SECTION("Copy Constructor"){
		testList list2(list);
		REQUIRE(list2.size==1);
		REQUIRE(list2[0] == 10);
		REQUIRE(list.maxSize()==260);
	};
	SECTION("Assignment copy"){
		testList list2;
		REQUIRE(list2.size==0);
		list2 = list;
		REQUIRE(list2.size==1);
		REQUIRE(list2[0] == 10);
		REQUIRE(list.maxSize()==260);
	};
	SECTION("Fill"){
		for(auto i=1;i<260;i++){
			REQUIRE(list.insert(i) == static_cast<int>(HasReturnvaluesIF::RETURN_OK));
		}
		REQUIRE(list.insert(260) == static_cast<int>(ArrayList<uint32_t, uint16_t>::FULL));
		list.clear();
		REQUIRE(list.size == 0);
	}
}



