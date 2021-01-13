#ifndef FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_
#define FSFW_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_

#include "LocalPoolDataSetBase.h"
#include "../datapool/SharedDataSetIF.h"
#include "../objectmanager/SystemObject.h"
#include <vector>

/**
 * This local dataset variation can be used if the dataset is used concurrently across
 * multiple threads. It provides a lock in addition to all other functionalities provided
 * by the LocalPoolDataSetBase class.
 *
 * TODO: override and protect read, commit and some other calls used by pool manager.
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
