#ifndef FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_
#define FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_

#include <vector>

#include "../datapool/SharedDataSetIF.h"
#include "../objectmanager/SystemObject.h"
#include "LocalPoolDataSetBase.h"

/**
 * This local dataset variation can be used if the dataset is used concurrently across
 * multiple threads. It provides a lock in addition to all other functionalities provided
 * by the LocalPoolDataSetBase class.
 *
 * The user is completely responsible for lockingand unlocking the dataset when using the
 * shared dataset.
 */
class SharedLocalDataSet : public SystemObject,
                           public LocalPoolDataSetBase,
                           public SharedDataSetIF {
 public:
  SharedLocalDataSet(object_id_t objectId, HasLocalDataPoolIF* owner, uint32_t setId,
                     const size_t maxSize);
  SharedLocalDataSet(object_id_t objectId, sid_t sid, const size_t maxSize);

  virtual ~SharedLocalDataSet();

  ReturnValue_t lockDataset(MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
                            dur_millis_t mutexTimeout = 20) override;
  ReturnValue_t unlockDataset() override;

 private:
  MutexIF* datasetLock = nullptr;
  std::vector<PoolVariableIF*> poolVarVector;
};

#endif /* FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_ */
