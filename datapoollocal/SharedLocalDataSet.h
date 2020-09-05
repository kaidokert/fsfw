#ifndef FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_
#define FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_

#include "LocalPoolDataSetBase.h"
#include "../datapool/SharedDataSetIF.h"
#include "../objectmanager/SystemObject.h"
#include <vector>

/**
 * Baseline question: If this dataset is shared, is there once instance
 * shared among many objects or multiple instances? Maybe be flexible
 * and provide both ways? Sharing one instance requires a mutex lock.
 * If there are multiple instances, it is not shared anymore, to be fair..
 * Then a regular local data set is sufficient.
 */
class SharedLocalDataSet: public SystemObject,
        public LocalPoolDataSetBase,
        public SharedDataSetIF {
public:
    SharedLocalDataSet(object_id_t objectId, sid_t sid,
            const size_t maxSize);
    ReturnValue_t lockDataset(dur_millis_t mutexTimeout) override;
    ReturnValue_t unlockDataset() override;
private:

    MutexIF* datasetLock = nullptr;
    std::vector<PoolVariableIF*> poolVarVector;
};



#endif /* FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_ */
