#ifndef FRAMEWORK_TEST_UNITTESTCLASS_H_
#define FRAMEWORK_TEST_UNITTESTCLASS_H_

#include "UnittDefinitions.h"
#include "fsfw/returnvalues/returnvalue.h"

/**
 * @brief	Can be used for internal testing, for example for hardware specific
 * 			tests which can not be run on a host-machine.
 *
 * TODO: A lot of ways to improve this class. A way for tests to subscribe
 * 		 in this central class would be nice. Right now, this is the class
 * 		 which simply calls all other tests from other files manually.
 * 		 Maybe there is a better way..
 */
class InternalUnitTester {
 public:
  struct TestConfig {
    bool testArrayPrinter = false;
    bool testSemaphores = true;
  };

  InternalUnitTester();
  virtual ~InternalUnitTester();

  /**
   * Some function which calls all other tests
   * @return
   */
  virtual ReturnValue_t performTests(const struct InternalUnitTester::TestConfig& testConfig);
};

#endif /* FRAMEWORK_TEST_UNITTESTCLASS_H_ */
