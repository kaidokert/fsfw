#ifndef FSFW_UNITTEST_TESTS_MOCKS_PERIODICTASKMOCK_H_
#define FSFW_UNITTEST_TESTS_MOCKS_PERIODICTASKMOCK_H_

#include <fsfw/tasks/ExecutableObjectIF.h>
#include <fsfw/tasks/PeriodicTaskBase.h>

class PeriodicTaskMock : public PeriodicTaskBase {
 public:
  PeriodicTaskMock(TaskPeriod period, TaskDeadlineMissedFunction dlmFunc)
      : PeriodicTaskBase(period, dlmFunc) {}

  virtual ~PeriodicTaskMock() {}
  /**
   * @brief	With the startTask method, a created task can be started
   *          for the first time.
   */
  virtual ReturnValue_t startTask() override {
    initObjsAfterTaskCreation();
    return returnvalue::OK;
  };

  virtual ReturnValue_t sleepFor(uint32_t ms) override { return returnvalue::OK; };
};

#endif  // FSFW_UNITTEST_TESTS_MOCKS_PERIODICTASKMOCK_H_
