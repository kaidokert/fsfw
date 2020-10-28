#include <fsfw/container/SimpleRingBuffer.h>
#include <catch2/catch.hpp>
#include "../../core/CatchDefinitions.h"

#include <cstring>

TEST_CASE("Ring Buffer Test" , "[RingBufferTest]") {
	uint8_t testData[13]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
	SimpleRingBuffer ringBuffer(10, false, 5);

	SECTION("Simple Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.writeData(testData, 3) == retval::CATCH_FAILED);
		REQUIRE(ringBuffer.readData(readBuffer, 5, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.availableWriteSpace() == 5);
		ringBuffer.clear();
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 4) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 4, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 4; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 9, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 9; i++) {
			CHECK(readBuffer[i] == i);
		}

	}

	SECTION("Get Free Element Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 8) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 1);
		REQUIRE(ringBuffer.readData(readBuffer, 8, true) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		uint8_t *testPtr = nullptr;
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) ==  retval::CATCH_FAILED);


		REQUIRE(ringBuffer.writeTillWrap() == 2);
		// too many excess bytes.
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) ==  retval::CATCH_FAILED);
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 5) ==  retval::CATCH_OK);
		REQUIRE(ringBuffer.getExcessBytes() == 3);
		std::memcpy(testPtr, testData, 5);
		ringBuffer.confirmBytesWritten(5);
		REQUIRE(ringBuffer.getAvailableReadData() == 5);
		ringBuffer.readData(readBuffer, 5, true);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
	}

	SECTION("Read Remaining Test") {
		REQUIRE(ringBuffer.writeData(testData, 3) == retval::CATCH_OK);
		REQUIRE(ringBuffer.getAvailableReadData() == 3);
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) ==  retval::CATCH_FAILED);
		size_t trueSize = 0;
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) ==  retval::CATCH_OK);
		REQUIRE(trueSize == 3);
		for(uint8_t i = 0; i< 3; i++) {
			CHECK(readBuffer[i] == i);
		}
		trueSize = 0;
		REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == retval::CATCH_FAILED);
		REQUIRE(trueSize == 0);
		REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == retval::CATCH_OK);
		REQUIRE(trueSize == 3);
	}
}

TEST_CASE("Ring Buffer Test2" , "[RingBufferTest2]") {
	uint8_t testData[13]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
	uint8_t* newBuffer = new uint8_t[10];
	SimpleRingBuffer ringBuffer(newBuffer, 10, true, 5);

	SECTION("Simple Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 5, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.availableWriteSpace() == 5);
		ringBuffer.clear();
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 4) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 4, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 4; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 9, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 9; i++) {
			CHECK(readBuffer[i] == i);
		}

	}

	SECTION("Get Free Element Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 8) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 1);
		REQUIRE(ringBuffer.readData(readBuffer, 8, true) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		uint8_t *testPtr = nullptr;
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) ==  retval::CATCH_FAILED);


		REQUIRE(ringBuffer.writeTillWrap() == 2);
		// too many excess bytes.
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) ==  retval::CATCH_FAILED);
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 5) ==  retval::CATCH_OK);
		REQUIRE(ringBuffer.getExcessBytes() == 3);
		std::memcpy(testPtr, testData, 5);
		ringBuffer.confirmBytesWritten(5);
		REQUIRE(ringBuffer.getAvailableReadData() == 5);
		ringBuffer.readData(readBuffer, 5, true);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
	}

	SECTION("Read Remaining Test") {
		REQUIRE(ringBuffer.writeData(testData, 3) == retval::CATCH_OK);
		REQUIRE(ringBuffer.getAvailableReadData() == 3);
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) ==  retval::CATCH_FAILED);
		size_t trueSize = 0;
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) ==  retval::CATCH_OK);
		REQUIRE(trueSize == 3);
		for(uint8_t i = 0; i< 3; i++) {
			CHECK(readBuffer[i] == i);
		}
		trueSize = 0;
		REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == retval::CATCH_FAILED);
		REQUIRE(trueSize == 0);
		REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == retval::CATCH_OK);
		REQUIRE(trueSize == 3);
	}

	SECTION("Overflow"){
		REQUIRE(ringBuffer.availableWriteSpace()==9);
		//Writing more than the buffer is large, technically thats allowed
		//But it is senseless and has undesired impact on read call
		REQUIRE(ringBuffer.writeData(testData, 13) == retval::CATCH_OK);
		REQUIRE(ringBuffer.getAvailableReadData()==3);
		ringBuffer.clear();
		uint8_t * ptr = nullptr;
		REQUIRE(ringBuffer.getFreeElement(&ptr, 13) == retval::CATCH_OK);
		REQUIRE(ptr != nullptr);
		memcpy(ptr, testData, 13);
		ringBuffer.confirmBytesWritten(13);
		REQUIRE(ringBuffer.getAvailableReadData()==3);
		REQUIRE(ringBuffer.readData(readBuffer, 3, true)== retval::CATCH_OK);
		for(auto i =0;i<3;i++){
			REQUIRE(readBuffer[i] == testData[i+10]);
		}
	}
}

TEST_CASE("Ring Buffer Test3" , "[RingBufferTest3]") {
	uint8_t testData[13]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
	uint8_t* newBuffer = new uint8_t[10];
	SimpleRingBuffer ringBuffer(newBuffer, 10, true, 15);

	SECTION("Simple Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 5, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.availableWriteSpace() == 5);
		ringBuffer.clear();
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 4) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 4, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 4; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 9, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 9; i++) {
			CHECK(readBuffer[i] == i);
		}

	}

	SECTION("Get Free Element Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 8) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 1);
		REQUIRE(ringBuffer.readData(readBuffer, 8, true) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		uint8_t *testPtr = nullptr;
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) ==  retval::CATCH_OK);
		REQUIRE(ringBuffer.getExcessBytes() == 8);

		REQUIRE(ringBuffer.writeTillWrap() == 2);
		// too many excess bytes.
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) ==  retval::CATCH_FAILED);
		// Less Execss bytes overwrites before
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 3) ==  retval::CATCH_OK);
		REQUIRE(ringBuffer.getExcessBytes() == 1);
		std::memcpy(testPtr, testData, 3);
		ringBuffer.confirmBytesWritten(3);
		REQUIRE(ringBuffer.getAvailableReadData() == 3);
		ringBuffer.readData(readBuffer, 3, true);
		for(uint8_t i = 0; i< 3; i++) {
			CHECK(readBuffer[i] == i);
		}
	}

	SECTION("Read Remaining Test") {
		REQUIRE(ringBuffer.writeData(testData, 3) == retval::CATCH_OK);
		REQUIRE(ringBuffer.getAvailableReadData() == 3);
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) ==  retval::CATCH_FAILED);
		size_t trueSize = 0;
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) ==  retval::CATCH_OK);
		REQUIRE(trueSize == 3);
		for(uint8_t i = 0; i< 3; i++) {
			CHECK(readBuffer[i] == i);
		}
		trueSize = 0;
		REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == retval::CATCH_FAILED);
		REQUIRE(trueSize == 0);
		REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == retval::CATCH_OK);
		REQUIRE(trueSize == 3);
	}

	SECTION("Overflow"){
		REQUIRE(ringBuffer.availableWriteSpace()==9);
		//Writing more than the buffer is large, technically thats allowed
		//But it is senseless and has undesired impact on read call
		REQUIRE(ringBuffer.writeData(testData, 13) == retval::CATCH_OK);
		REQUIRE(ringBuffer.getAvailableReadData()==3);
		ringBuffer.clear();
		uint8_t * ptr = nullptr;
		REQUIRE(ringBuffer.getFreeElement(&ptr, 13) == retval::CATCH_OK);
		REQUIRE(ptr != nullptr);
		memcpy(ptr, testData, 13);
		ringBuffer.confirmBytesWritten(13);
		REQUIRE(ringBuffer.getAvailableReadData()==3);
		REQUIRE(ringBuffer.readData(readBuffer, 3, true)== retval::CATCH_OK);
		for(auto i =0;i<3;i++){
			REQUIRE(readBuffer[i] == testData[i+10]);
		}
	}
}

TEST_CASE("Ring Buffer Test4" , "[RingBufferTest4]") {
	uint8_t testData[13]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	uint8_t readBuffer[10] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
	SimpleRingBuffer ringBuffer(10, false, 15);

	SECTION("Simple Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.writeData(testData, 3) == retval::CATCH_FAILED);
		REQUIRE(ringBuffer.readData(readBuffer, 5, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.availableWriteSpace() == 5);
		ringBuffer.clear();
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 4) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 4, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 4; i++) {
			CHECK(readBuffer[i] == i);
		}
		REQUIRE(ringBuffer.writeData(testData, 9) == retval::CATCH_OK);
		REQUIRE(ringBuffer.readData(readBuffer, 9, true) == retval::CATCH_OK);
		for(uint8_t i = 0; i< 9; i++) {
			CHECK(readBuffer[i] == i);
		}

	}

	SECTION("Get Free Element Test") {
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		REQUIRE(ringBuffer.writeData(testData, 8) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 1);
		REQUIRE(ringBuffer.readData(readBuffer, 8, true) == retval::CATCH_OK);
		REQUIRE(ringBuffer.availableWriteSpace() == 9);
		uint8_t *testPtr = nullptr;
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 10) ==  retval::CATCH_FAILED);


		REQUIRE(ringBuffer.writeTillWrap() == 2);
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 8) ==  retval::CATCH_OK);
		REQUIRE(ringBuffer.getFreeElement(&testPtr, 5) ==  retval::CATCH_OK);
		REQUIRE(ringBuffer.getExcessBytes() == 3);
		std::memcpy(testPtr, testData, 5);
		ringBuffer.confirmBytesWritten(5);
		REQUIRE(ringBuffer.getAvailableReadData() == 5);
		ringBuffer.readData(readBuffer, 5, true);
		for(uint8_t i = 0; i< 5; i++) {
			CHECK(readBuffer[i] == i);
		}
	}

	SECTION("Read Remaining Test") {
		REQUIRE(ringBuffer.writeData(testData, 3) == retval::CATCH_OK);
		REQUIRE(ringBuffer.getAvailableReadData() == 3);
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, false, nullptr) ==  retval::CATCH_FAILED);
		size_t trueSize = 0;
		REQUIRE(ringBuffer.readData(readBuffer, 5, false, true, &trueSize) ==  retval::CATCH_OK);
		REQUIRE(trueSize == 3);
		for(uint8_t i = 0; i< 3; i++) {
			CHECK(readBuffer[i] == i);
		}
		trueSize = 0;
		REQUIRE(ringBuffer.deleteData(5, false, &trueSize) == retval::CATCH_FAILED);
		REQUIRE(trueSize == 0);
		REQUIRE(ringBuffer.deleteData(5, true, &trueSize) == retval::CATCH_OK);
		REQUIRE(trueSize == 3);
	}
}
