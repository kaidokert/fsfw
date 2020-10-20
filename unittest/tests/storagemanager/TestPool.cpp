#include <config/objects/Factory.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/storagemanager/LocalPool.h>
#include <catch2/catch.hpp>
#include "core/CatchDefinitions.h"

TEST_CASE( "Local Pool Simple Tests [1 Pool]" , "[TestPool]") {
	uint16_t numberOfElements[1] = {1};
	uint16_t sizeofElements[1] = {10};
	LocalPool<1> SimplePool = LocalPool<1>(0, sizeofElements, numberOfElements);
	std::array<uint8_t, 20> testDataArray;
	std::array<uint8_t, 20> receptionArray;
	store_address_t testStoreId;
	ReturnValue_t result = retval::CATCH_FAILED;
	uint8_t *pointer = nullptr;
	const uint8_t * constPointer = nullptr;
	uint8_t test = 0;

	for(size_t i = 0; i < testDataArray.size(); i++) {
		testDataArray[i] = i;
	}
	size_t size = 10;

	SECTION ( "Basic tests") {
		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
		REQUIRE(result == retval::CATCH_OK);
		result = SimplePool.getData(testStoreId, &constPointer, &size);
		REQUIRE(result == retval::CATCH_OK);
		memcpy(receptionArray.data(), constPointer, size);
		for(size_t i = 0; i < size; i++) {
			CHECK(receptionArray[i] == i );
		}
		memset(receptionArray.data(), 0, size);
		result = SimplePool.modifyData(testStoreId, &pointer, &size);
		memcpy(receptionArray.data(), pointer, size);
		REQUIRE(result == retval::CATCH_OK);
		for(size_t i = 0; i < size; i++) {
			CHECK(receptionArray[i] == i );
		}
		result = SimplePool.deleteData(testStoreId);
		REQUIRE(result == retval::CATCH_OK);
		result = SimplePool.addData(&testStoreId, testDataArray.data(), 15);
		CHECK (result == (int) StorageManagerIF::DATA_TOO_LARGE);
	}

	SECTION ( "Reservation Tests ") {
		pointer = nullptr;
		result = SimplePool.getFreeElement(&testStoreId, size, &pointer);
		REQUIRE (result == retval::CATCH_OK);
		memcpy(pointer, testDataArray.data(), size);
		constPointer = nullptr;
		result = SimplePool.getData(testStoreId, &constPointer, &size);

		REQUIRE (result == retval::CATCH_OK);
		memcpy(receptionArray.data(), constPointer, size);
		for(size_t i = 0; i < size; i++) {
			CHECK(receptionArray[i] == i );
		}
	}

	SECTION ( "Add, delete, add, add when full") {
		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
		REQUIRE(result == retval::CATCH_OK);
		result = SimplePool.getData(testStoreId, &constPointer, &size);
		REQUIRE( result == retval::CATCH_OK);
		memcpy(receptionArray.data(), constPointer, size);
		for(size_t i = 0; i < size; i++) {
			CHECK(receptionArray[i] == i );
		}

		result = SimplePool.deleteData(testStoreId);
		REQUIRE(result == retval::CATCH_OK);

		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
		REQUIRE(result == retval::CATCH_OK);
		result = SimplePool.getData(testStoreId, &constPointer, &size);
		REQUIRE( result == retval::CATCH_OK);
		memcpy(receptionArray.data(), constPointer, size);
		for(size_t i = 0; i < size; i++) {
			CHECK(receptionArray[i] == i );
		}

		store_address_t newAddress;
		result = SimplePool.addData(&newAddress, testDataArray.data(), size);
		REQUIRE(result == (int) StorageManagerIF::DATA_STORAGE_FULL);
	}

	SECTION ( "Initialize and clear store, delete with pointer") {
		result = SimplePool.initialize();
		REQUIRE(result == retval::CATCH_OK);
		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
		REQUIRE(result == retval::CATCH_OK);
		SimplePool.clearStore();
		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
		REQUIRE(result == retval::CATCH_OK);
		result = SimplePool.modifyData(testStoreId, &pointer, &size);
		REQUIRE(result == retval::CATCH_OK);
		store_address_t newId;
		result = SimplePool.deleteData(pointer, size, &testStoreId);
		REQUIRE(result == retval::CATCH_OK);
		REQUIRE(testStoreId.raw != (uint32_t) StorageManagerIF::INVALID_ADDRESS);
		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
		REQUIRE(result == retval::CATCH_OK);
	}
}
