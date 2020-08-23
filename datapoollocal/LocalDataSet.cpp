#include "LocalDataSet.h"
#include "../datapoollocal/LocalDataPoolManager.h"
#include "../serialize/SerializeAdapter.h"

#include <cmath>
#include <cstring>

LocalDataSet::LocalDataSet(HasLocalDataPoolIF *hkOwner,
        const size_t maxNumberOfVariables):
        LocalPoolDataSetBase(hkOwner,poolVarList.data(), maxNumberOfVariables) {
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
        LocalPoolDataSetBase(ownerId, poolVarList.data(), maxNumberOfVariables)  {
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

