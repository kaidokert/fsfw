#include <unittest/internal/InternalUnitTester.h>
#include <unittest/internal/IntTestMq.h>
#include <unittest/internal/IntTestSemaphore.h>
#include <unittest/internal/IntTestSerialization.h>
#include <unittest/internal/UnittDefinitions.h>
#include <unittest/internal/IntTestMutex.h>

#include <cstdlib>

InternalUnitTester::InternalUnitTester() {}

InternalUnitTester::~InternalUnitTester() {}

ReturnValue_t InternalUnitTester::performTests() {
	sif::info << "Running internal unit tests..\r\n" << std::flush;
	testserialize::test_serialization();
	testmq::testMq();
	testsemaph::testBinSemaph();
	testsemaph::testCountingSemaph();
	testmutex::testMutex();
	sif::info << "Internal unit tests finished.\r\n" << std::flush;
	return RETURN_OK;
}



