#include "fsfw_tests/internal/InternalUnitTester.h"

#include <cstdlib>

#include "fsfw/serviceinterface.h"
#include "fsfw_tests/internal/UnittDefinitions.h"
#include "fsfw_tests/internal/globalfunctions/TestArrayPrinter.h"
#include "fsfw_tests/internal/osal/testMq.h"
#include "fsfw_tests/internal/osal/testMutex.h"
#include "fsfw_tests/internal/osal/testSemaphore.h"
#include "fsfw_tests/internal/serialize/IntTestSerialization.h"

InternalUnitTester::InternalUnitTester() {}

InternalUnitTester::~InternalUnitTester() {}

ReturnValue_t InternalUnitTester::performTests(
    const struct InternalUnitTester::TestConfig& testConfig) {
  FSFW_LOGI("Running internal unit tests.. Error messages might follow\n");

  testserialize::test_serialization();
  testmq::testMq();
  if (testConfig.testSemaphores) {
    testsemaph::testBinSemaph();
    testsemaph::testCountingSemaph();
  }
  testmutex::testMutex();
  if (testConfig.testArrayPrinter) {
    arrayprinter::testArrayPrinter();
  }

  FSFW_LOGI("Internal unit tests finished\n");
  return RETURN_OK;
}
