//#include <fsfw/storagemanager/LocalPool.h>
//#include <catch2/catch.hpp>
//#include "../../core/CatchDefinitions.h"
//#include <array>
//
//TEST_CASE( "New Accessor" , "[NewAccessor]") {
//	uint16_t numberOfElements[1] = {1};
//	uint16_t sizeofElements[1] = {10};
//	LocalPool<1> SimplePool = LocalPool<1>(0, sizeofElements, numberOfElements);
//	std::array<uint8_t, 20> testDataArray;
//	std::array<uint8_t, 20> receptionArray;
//	store_address_t testStoreId;
//	ReturnValue_t result = retval::CATCH_FAILED;
//
//	for(size_t i = 0; i < testDataArray.size(); i++) {
//		testDataArray[i] = i;
//	}
//	size_t size = 10;
//
//	SECTION ("Simple tests getter functions") {
//		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
//		REQUIRE(result == retval::CATCH_OK);
//		auto resultPair = SimplePool.getData(testStoreId);
//		REQUIRE(resultPair.first == retval::CATCH_OK);
//		resultPair.second.getDataCopy(receptionArray.data(), 20);
//		CHECK(resultPair.second.getId() == testStoreId);
//		CHECK(resultPair.second.size() == 10);
//		for(size_t i = 0; i < size; i++) {
//			CHECK(receptionArray[i] == i );
//		}
//
//		std::copy(resultPair.second.data(), resultPair.second.data() +
//				resultPair.second.size(), receptionArray.data());
//		for(size_t i = 0; i < size; i++) {
//			CHECK(receptionArray[i] == i );
//		}
//
//		{
//			auto resultPairLoc = SimplePool.getData(testStoreId);
//			REQUIRE(resultPairLoc.first == retval::CATCH_OK);
//			// data should be deleted when accessor goes out of scope.
//		}
//		resultPair = SimplePool.getData(testStoreId);
//		REQUIRE(resultPair.first == (int) StorageManagerIF::DATA_DOES_NOT_EXIST);
//
//		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
//		REQUIRE(result == retval::CATCH_OK);
//		{
//			ConstStorageAccessor constAccessor(testStoreId);
//			result = SimplePool.getData(testStoreId, constAccessor);
//			REQUIRE(result == retval::CATCH_OK);
//			constAccessor.getDataCopy(receptionArray.data(), 20);
//			for(size_t i = 0; i < size; i++) {
//				CHECK(receptionArray[i] == i );
//			}
//			// likewise, data should be deleted when accessor gets out of scope.
//		}
//		resultPair = SimplePool.getData(testStoreId);
//		REQUIRE(resultPair.first == (int) StorageManagerIF::DATA_DOES_NOT_EXIST);
//
//		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
//		{
//			resultPair = SimplePool.getData(testStoreId);
//			REQUIRE(resultPair.first == retval::CATCH_OK);
//			resultPair.second.release();
//			// now data should not be deleted anymore
//		}
//		resultPair = SimplePool.getData(testStoreId);
//		REQUIRE(resultPair.first == retval::CATCH_OK);
//		resultPair.second.getDataCopy(receptionArray.data(), 20);
//		for(size_t i = 0; i < size; i++) {
//			CHECK(receptionArray[i] == i );
//		}
//	}
//
//
//	SECTION("Simple tests modify functions") {
//		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
//		REQUIRE(result == retval::CATCH_OK);
//		{
//			StorageAccessor accessor(testStoreId);
//			result = SimplePool.modifyData(testStoreId, accessor);
//			REQUIRE(result == retval::CATCH_OK);
//			CHECK(accessor.getId() == testStoreId);
//			CHECK(accessor.size() == 10);
//			accessor.getDataCopy(receptionArray.data(), 20);
//			for(size_t i = 0; i < size; i++) {
//				CHECK(receptionArray[i] == i );
//			}
//			std::copy(accessor.data(), accessor.data() +
//					accessor.size(), receptionArray.data());
//			for(size_t i = 0; i < size; i++) {
//				CHECK(receptionArray[i] == i );
//			}
//			// data should be deleted when accessor goes out of scope
//		}
//		auto resultPair = SimplePool.getData(testStoreId);
//		REQUIRE(resultPair.first == (int) StorageManagerIF::DATA_DOES_NOT_EXIST);
//
//		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
//		REQUIRE(result == retval::CATCH_OK);
//		{
//			auto resultPairLoc = SimplePool.modifyData(testStoreId);
//			REQUIRE(resultPairLoc.first == retval::CATCH_OK);
//			CHECK(resultPairLoc.second.getId() == testStoreId);
//			CHECK(resultPairLoc.second.size() == 10);
//			resultPairLoc.second.getDataCopy(receptionArray.data(), 20);
//			for(size_t i = 0; i < size; i++) {
//				CHECK(receptionArray[i] == i );
//			}
//			std::copy(resultPairLoc.second.data(), resultPairLoc.second.data() +
//					resultPairLoc.second.size(), receptionArray.data());
//			for(size_t i = 0; i < size; i++) {
//				CHECK(receptionArray[i] == i );
//			}
//			resultPairLoc.second.release();
//			// data should not be deleted when accessor goes out of scope
//		}
//		resultPair = SimplePool.getData(testStoreId);
//		REQUIRE(resultPair.first == retval::CATCH_OK);
//	}
//
//
//	SECTION("Write tests") {
//		result = SimplePool.addData(&testStoreId, testDataArray.data(), size);
//		REQUIRE(result == retval::CATCH_OK);
//		{
//			auto resultPair = SimplePool.modifyData(testStoreId);
//			REQUIRE(resultPair.first == retval::CATCH_OK);
//			testDataArray[9] = 42;
//			resultPair.second.write(testDataArray.data(), 10, 0);
//			// now data should not be deleted
//			resultPair.second.release();
//		}
//		auto resultConstPair = SimplePool.getData(testStoreId);
//		REQUIRE(resultConstPair.first == retval::CATCH_OK);
//
//		resultConstPair.second.getDataCopy(receptionArray.data(), 10);
//		for(size_t i = 0; i < size-1; i++) {
//			CHECK(receptionArray[i] == i );
//		}
//		CHECK(receptionArray[9] == 42 );
//
//		auto resultPair = SimplePool.modifyData(testStoreId);
//		REQUIRE(resultPair.first == retval::CATCH_OK);
//		result = resultPair.second.write(testDataArray.data(), 20, 0);
//		REQUIRE(result == retval::CATCH_FAILED);
//		result = resultPair.second.write(testDataArray.data(), 10, 5);
//		REQUIRE(result == retval::CATCH_FAILED);
//
//		memset(testDataArray.data(), 42, 5);
//		result = resultPair.second.write(testDataArray.data(), 5, 5);
//		REQUIRE(result == retval::CATCH_OK);
//		resultConstPair = SimplePool.getData(testStoreId);
//		resultPair.second.getDataCopy(receptionArray.data(), 20);
//		for(size_t i = 5; i < 10; i++) {
//			CHECK(receptionArray[i] == 42 );
//		}
//
//	}
//}
