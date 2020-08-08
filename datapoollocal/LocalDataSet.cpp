#include <framework/datapoollocal/LocalDataPoolManager.h>
#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/serialize/SerializeAdapter.h>

#include <cmath>
#include <cstring>

LocalDataSet::LocalDataSet(HasLocalDataPoolIF *hkOwner,
        const size_t maxNumberOfVariables):
        LocalDataSetBase(hkOwner,poolVarList.data(), maxNumberOfVariables) {
    poolVarList.reserve(maxNumberOfVariables);
    poolVarList.resize(maxNumberOfVariables);
    if(hkOwner == nullptr) {
        sif::error << "LocalDataSet::LocalDataSet: Owner can't be nullptr!"
                << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
}

LocalDataSet::LocalDataSet(object_id_t ownerId,
        const size_t maxNumberOfVariables):
        LocalDataSetBase(ownerId, poolVarList.data(), maxNumberOfVariables)  {
    poolVarList.reserve(maxNumberOfVariables);
    poolVarList.resize(maxNumberOfVariables);
    HasLocalDataPoolIF* hkOwner = objectManager->get<HasLocalDataPoolIF>(
            ownerId);
    if(hkOwner == nullptr) {
        sif::error << "LocalDataSet::LocalDataSet: Owner can't be nullptr!"
                << std::endl;
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
}

LocalDataSet::~LocalDataSet() {}

