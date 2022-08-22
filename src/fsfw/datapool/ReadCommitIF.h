#ifndef FSFW_DATAPOOL_READCOMMITIF_H_
#define FSFW_DATAPOOL_READCOMMITIF_H_

#include "../ipc/MutexIF.h"
#include "../returnvalues/returnvalue.h"

/**
 * @brief 	Common interface for all software objects which employ read-commit
 * 			semantics.
 */
class ReadCommitIF {
  friend class ReadCommitIFAttorney;

 public:
  virtual ~ReadCommitIF() {}
  virtual ReturnValue_t read(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs) = 0;
  virtual ReturnValue_t commit(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs) = 0;

 protected:
  /* Optional and protected because this is interesting for classes grouping members with commit
  and read semantics where the lock is only necessary once. */
  virtual ReturnValue_t readWithoutLock() { return read(MutexIF::TimeoutType::WAITING, 20); }

  virtual ReturnValue_t commitWithoutLock() { return commit(MutexIF::TimeoutType::WAITING, 20); }
};

#endif /* FSFW_DATAPOOL_READCOMMITIF_H_ */
