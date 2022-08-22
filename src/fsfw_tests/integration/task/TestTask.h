#ifndef MISSION_DEMO_TESTTASK_H_
#define MISSION_DEMO_TESTTASK_H_

#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/storagemanager/StorageManagerIF.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

/**
 * @brief 	Test class for general C++ testing and any other code which will not be part of the
 *          primary mission software.
 * @details
 * Should not be used for board specific tests. Instead, a derived board test class should be used.
 */
class TestTask : public SystemObject, public ExecutableObjectIF {
 public:
  explicit TestTask(object_id_t objectId);
  ~TestTask() override;
  ReturnValue_t performOperation(uint8_t operationCode) override;

 protected:
  virtual ReturnValue_t performOneShotAction();
  virtual ReturnValue_t performPeriodicAction();
  virtual ReturnValue_t performActionA();
  virtual ReturnValue_t performActionB();

  enum testModes : uint8_t { A, B };

  testModes testMode;
  bool testFlag = false;

 private:
  bool oneShotAction = true;
  static MutexIF* testLock;
  StorageManagerIF* IPCStore;
};

#endif /* TESTTASK_H_ */
