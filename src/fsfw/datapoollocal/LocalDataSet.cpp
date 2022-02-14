#include "fsfw/datapoollocal/LocalDataSet.h"

#include <cmath>
#include <cstring>

#include "fsfw/datapoollocal/LocalDataPoolManager.h"
#include "fsfw/serialize/SerializeAdapter.h"

LocalDataSet::LocalDataSet(HasLocalDataPoolIF *hkOwner, uint32_t setId,
                           const size_t maxNumberOfVariables)
    : LocalPoolDataSetBase(hkOwner, setId, nullptr, maxNumberOfVariables),
      poolVarList(maxNumberOfVariables) {
  this->setContainer(poolVarList.data());
}

LocalDataSet::LocalDataSet(sid_t sid, const size_t maxNumberOfVariables)
    : LocalPoolDataSetBase(sid, nullptr, maxNumberOfVariables), poolVarList(maxNumberOfVariables) {
  this->setContainer(poolVarList.data());
}

LocalDataSet::~LocalDataSet() {}
