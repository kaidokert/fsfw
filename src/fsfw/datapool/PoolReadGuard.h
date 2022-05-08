#ifndef FSFW_DATAPOOL_POOLREADHELPER_H_
#define FSFW_DATAPOOL_POOLREADHELPER_H_

#include "ReadCommitIF.h"
#include "fsfw/FSFW.h"
#include "fsfw/serviceinterface.h"

/**
 * @brief 	Helper class to read data sets or pool variables
 */
class PoolReadGuard {
 public:
  PoolReadGuard(ReadCommitIF* readObject,
                MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
                uint32_t mutexTimeout = 20)
      : readObject(readObject), mutexTimeout(mutexTimeout) {
    if (readObject != nullptr) {
      readResult = readObject->read(timeoutType, mutexTimeout);
      if (readResult != HasReturnvaluesIF::RETURN_OK) {
        FSFW_LOGW("{}", "ctor: Read failed\n");
      }
    }
  }

  ReturnValue_t getReadResult() const { return readResult; }

  /**
   * @brief   Can be used to suppress commit on destruction.
   */
  void setNoCommitMode(bool commit) { this->noCommit = commit; }

  /**
   * @brief   Default destructor which will take care of commiting changed values.
   */
  ~PoolReadGuard() {
    if (readObject != nullptr and not noCommit) {
      readObject->commit(timeoutType, mutexTimeout);
    }
  }

 private:
  ReadCommitIF* readObject = nullptr;
  ReturnValue_t readResult = HasReturnvaluesIF::RETURN_OK;
  bool noCommit = false;
  MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
  uint32_t mutexTimeout = 20;
};

#endif /* FSFW_DATAPOOL_POOLREADHELPER_H_ */
