#ifndef FSFW_DATAPOOL_POOLREADHELPER_H_
#define FSFW_DATAPOOL_POOLREADHELPER_H_

#include <FSFWConfig.h>

#include "../serviceinterface/ServiceInterface.h"
#include "ReadCommitIF.h"

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
      if (readResult != returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "PoolReadHelper: Read failed!" << std::endl;
#else
        sif::printError("PoolReadHelper: Read failed!\n");
#endif /* FSFW_PRINT_VERBOSITY_LEVEL == 1 */
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
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
  ReturnValue_t readResult = returnvalue::OK;
  bool noCommit = false;
  MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING;
  uint32_t mutexTimeout = 20;
};

#endif /* FSFW_DATAPOOL_POOLREADHELPER_H_ */
