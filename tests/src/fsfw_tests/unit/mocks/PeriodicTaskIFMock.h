#ifndef FSFW_UNITTEST_TESTS_MOCKS_PERIODICTASKMOCK_H_
#define FSFW_UNITTEST_TESTS_MOCKS_PERIODICTASKMOCK_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/tasks/PeriodicTaskIF.h>

class PeriodicTaskMock : public PeriodicTaskIF {
 public:
  PeriodicTaskMock(uint32_t period = 5) : period(period) {}
  /**
   * @brief	A virtual destructor as it is mandatory for interfaces.
   */
  virtual ~PeriodicTaskMock() {}
  /**
   * @brief	With the startTask method, a created task can be started
   *          for the first time.
   */
  virtual ReturnValue_t startTask() override { return HasReturnvaluesIF::RETURN_OK; };

  virtual ReturnValue_t addComponent(object_id_t object) override {
    ExecutableObjectIF* executableObject =
        ObjectManager::instance()->get<ExecutableObjectIF>(objects::INTERNAL_ERROR_REPORTER);
    if (executableObject == nullptr) {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
    executableObject->setTaskIF(this);
    executableObject->initializeAfterTaskCreation();
    return HasReturnvaluesIF::RETURN_OK;
  };

  virtual ReturnValue_t sleepFor(uint32_t ms) override { return HasReturnvaluesIF::RETURN_OK; };

  virtual uint32_t getPeriodMs() const override { return period; };
  uint32_t period;
};

#endif  // FSFW_UNITTEST_TESTS_MOCKS_PERIODICTASKMOCK_H_