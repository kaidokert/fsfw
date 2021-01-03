#include "InternalUnitTester.h"
#include "UnittDefinitions.h"

#include "osal/IntTestMq.h"
#include "osal/IntTestSemaphore.h"
#include "osal/IntTestMutex.h"
#include "serialize/IntTestSerialization.h"

#include <cstdlib>

InternalUnitTester::InternalUnitTester() {}

InternalUnitTester::~InternalUnitTester() {}

ReturnValue_t InternalUnitTester::performTests() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::info << "Running internal unit tests.." << std::endl;
#endif
	testserialize::test_serialization();
	testmq::testMq();
	testsemaph::testBinSemaph();
	testsemaph::testCountingSemaph();
	testmutex::testMutex();
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::info << "Internal unit tests finished." << std::endl;
#endif
	return RETURN_OK;
}



