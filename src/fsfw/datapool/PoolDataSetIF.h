#ifndef FSFW_DATAPOOL_POOLDATASETIF_H_
#define FSFW_DATAPOOL_POOLDATASETIF_H_

#include "DataSetIF.h"
#include "ReadCommitIF.h"

/**
 * @brief   Extendes the DataSetIF by adding abstract functions to lock
 *          and unlock a data pool and read/commit semantics.
 */
class PoolDataSetIF : virtual public DataSetIF, virtual public ReadCommitIF {
 public:
  virtual ~PoolDataSetIF(){};

  /**
   * @brief   Most underlying data structures will have a pool like structure
   *          and will require a lock and unlock mechanism to ensure
   *          thread-safety
   * @return Lock operation result
   */
  virtual ReturnValue_t lockDataPool(
      MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
      uint32_t timeoutMs = 20) = 0;

  /**
   * @brief   Unlock call corresponding to the lock call.
   * @return Unlock operation result
   */
  virtual ReturnValue_t unlockDataPool() = 0;

  virtual bool isValid() const = 0;
};

#endif /* FSFW_DATAPOOL_POOLDATASETIF_H_ */
