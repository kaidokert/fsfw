#ifndef FRAMEWORK_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_
#define FRAMEWORK_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_
#include <framework/datapool/SharedDataSetIF.h>
#include <framework/datapoollocal/LocalPoolDataSetBase.h>
#include <framework/objectmanager/SystemObject.h>
#include <vector>

class SharedLocalDataSet: public SystemObject,
        public LocalDataSetBase,
        public SharedDataSetIF {
public:
    SharedLocalDataSet(object_id_t objectId, object_id_t owner,
            const size_t maxSize);
    ReturnValue_t lockDataset(dur_millis_t mutexTimeout) override;
    ReturnValue_t unlockDataset() override;
private:


    MutexIF* datasetLock = nullptr;
    std::vector<PoolVariableIF*> poolVarVector;
};



#endif /* FRAMEWORK_DATAPOOLLOCAL_SHAREDLOCALDATASET_H_ */
