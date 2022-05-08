#include "fsfw/datapoollocal/LocalPoolObjectBase.h"

#include "fsfw/datapoollocal/AccessLocalPoolF.h"
#include "fsfw/datapoollocal/HasLocalDataPoolIF.h"
#include "fsfw/datapoollocal/LocalDataPoolManager.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface.h"
#include "internal/HasLocalDpIFUserAttorney.h"

LocalPoolObjectBase::LocalPoolObjectBase(lp_id_t poolId, HasLocalDataPoolIF* hkOwner,
                                         DataSetIF* dataSet, pool_rwm_t setReadWriteMode)
    : localPoolId(poolId), readWriteMode(setReadWriteMode) {
  if (poolId == PoolVariableIF::NO_PARAMETER) {
    FSFW_LOGWT("{}", "ctor: Invalid pool ID, has NO_PARAMETER value\n");
  }
  if (hkOwner == nullptr) {
    FSFW_LOGET("{}", "ctor: Supplied pool owner is a invalid\n");
    return;
  }
  AccessPoolManagerIF* poolManAccessor = HasLocalDpIFUserAttorney::getAccessorHandle(hkOwner);
  hkManager = poolManAccessor->getPoolManagerHandle();

  if (dataSet != nullptr) {
    dataSet->registerVariable(this);
  }
}

LocalPoolObjectBase::LocalPoolObjectBase(object_id_t poolOwner, lp_id_t poolId, DataSetIF* dataSet,
                                         pool_rwm_t setReadWriteMode)
    : localPoolId(poolId), readWriteMode(setReadWriteMode) {
  if (poolId == PoolVariableIF::NO_PARAMETER) {
    FSFW_LOGWT("{}", "ctor: Invalid pool ID, has NO_PARAMETER value\n");
  }
  auto* hkOwner = ObjectManager::instance()->get<HasLocalDataPoolIF>(poolOwner);
  if (hkOwner == nullptr) {
    FSFW_LOGWT(
        "ctor: The supplied pool owner {:#010x} did not implement the correct interface "
        "HasLocalDataPoolIF\n",
        poolOwner);
    return;
  }

  AccessPoolManagerIF* accessor = HasLocalDpIFUserAttorney::getAccessorHandle(hkOwner);
  if (accessor != nullptr) {
    hkManager = accessor->getPoolManagerHandle();
  }

  if (dataSet != nullptr) {
    dataSet->registerVariable(this);
  }
}

pool_rwm_t LocalPoolObjectBase::getReadWriteMode() const { return readWriteMode; }

bool LocalPoolObjectBase::isValid() const { return valid; }

void LocalPoolObjectBase::setValid(bool valid) { this->valid = valid; }

lp_id_t LocalPoolObjectBase::getDataPoolId() const { return localPoolId; }

void LocalPoolObjectBase::setDataPoolId(lp_id_t poolId) { this->localPoolId = poolId; }

void LocalPoolObjectBase::setChanged(bool changed) { this->changed = changed; }

bool LocalPoolObjectBase::hasChanged() const { return changed; }

void LocalPoolObjectBase::setReadWriteMode(pool_rwm_t newReadWriteMode) {
  this->readWriteMode = newReadWriteMode;
}

void LocalPoolObjectBase::reportReadCommitError(const char* variableType, ReturnValue_t error,
                                                bool read, object_id_t objectId, lp_id_t lpId) {
  const char* variablePrintout = variableType;
  if (variablePrintout == nullptr) {
    variablePrintout = "Unknown Type";
  }
  const char* type = nullptr;
  if (read) {
    type = "read";
  } else {
    type = "commit";
  }

  const char* errMsg = nullptr;
  if (error == localpool::POOL_ENTRY_NOT_FOUND) {
    errMsg = "Pool entry not found";
  } else if (error == localpool::POOL_ENTRY_TYPE_CONFLICT) {
    errMsg = "Pool entry type conflict";
  } else if (error == PoolVariableIF::INVALID_READ_WRITE_MODE) {
    errMsg = "Pool variable wrong read-write mode";
  } else if (error == PoolVariableIF::INVALID_POOL_ENTRY) {
    errMsg = "Pool entry invalid";
  } else {
    errMsg = "Unknown error code";
  }

  FSFW_LOGW("{}: {} call | {} | Owner: {:#010x} | LPID: \n", variablePrintout, type, errMsg,
            objectId, lpId);
}
