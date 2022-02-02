#ifndef FRAMEWORK_DATAPOOL_SHAREDDATASETIF_H_
#define FRAMEWORK_DATAPOOL_SHAREDDATASETIF_H_

#include "PoolDataSetIF.h"

class SharedDataSetIF {
 public:
  virtual ~SharedDataSetIF(){};

 private:
  virtual ReturnValue_t lockDataset(MutexIF::TimeoutType timeoutType,
                                    dur_millis_t mutexTimeout) = 0;
  virtual ReturnValue_t unlockDataset() = 0;
};

#endif /* FRAMEWORK_DATAPOOL_SHAREDDATASETIF_H_ */
