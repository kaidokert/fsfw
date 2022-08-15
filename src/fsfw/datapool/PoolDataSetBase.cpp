#include "fsfw/datapool/PoolDataSetBase.h"

#include <cstring>

#include "fsfw/datapool/ReadCommitIFAttorney.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PoolDataSetBase::PoolDataSetBase(PoolVariableIF** registeredVariablesArray,
                                 const size_t maxFillCount)
    : registeredVariables(registeredVariablesArray), maxFillCount(maxFillCount) {}

PoolDataSetBase::~PoolDataSetBase() {}

ReturnValue_t PoolDataSetBase::registerVariable(PoolVariableIF* variable) {
  if (registeredVariables == nullptr) {
    /* Underlying container invalid */
    return returnvalue::FAILED;
  }
  if (state != States::STATE_SET_UNINITIALISED) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DataSet::registerVariable: Call made in wrong position." << std::endl;
#else
    sif::printError("DataSet::registerVariable: Call made in wrong position.");
#endif
    return DataSetIF::DATA_SET_UNINITIALISED;
  }
  if (variable == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DataSet::registerVariable: Pool variable is nullptr." << std::endl;
#else
    sif::printError("DataSet::registerVariable: Pool variable is nullptr.\n");
#endif
    return DataSetIF::POOL_VAR_NULL;
  }
  if (fillCount >= maxFillCount) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DataSet::registerVariable: DataSet is full." << std::endl;
#else
    sif::printError("DataSet::registerVariable: DataSet is full.\n");
#endif
    return DataSetIF::DATA_SET_FULL;
  }
  registeredVariables[fillCount] = variable;
  fillCount++;
  return returnvalue::OK;
}

ReturnValue_t PoolDataSetBase::read(MutexIF::TimeoutType timeoutType, uint32_t lockTimeout) {
  ReturnValue_t result = returnvalue::OK;
  ReturnValue_t error = result;
  if (state == States::STATE_SET_UNINITIALISED) {
    lockDataPool(timeoutType, lockTimeout);
    for (uint16_t count = 0; count < fillCount; count++) {
      result = readVariable(count);
      if (result != returnvalue::OK) {
        error = result;
      }
    }
    state = States::STATE_SET_WAS_READ;
    unlockDataPool();
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "PoolDataSetBase::read: Call made in wrong position. Don't forget to "
                    "commit member datasets!"
                 << std::endl;
#else
    sif::printWarning(
        "PoolDataSetBase::read: Call made in wrong position. Don't forget to "
        "commit member datasets!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
    result = SET_WAS_ALREADY_READ;
  }

  if (error != returnvalue::OK) {
    result = error;
  }
  return result;
}

uint16_t PoolDataSetBase::getFillCount() const { return fillCount; }

ReturnValue_t PoolDataSetBase::readVariable(uint16_t count) {
  ReturnValue_t result = returnvalue::OK;
  if (registeredVariables[count] == nullptr) {
    /* Configuration error. */
    return returnvalue::FAILED;
  }

  /* These checks are often performed by the respective variable implementation too, but I guess
  a double check does not hurt. */
  if (registeredVariables[count]->getReadWriteMode() != PoolVariableIF::VAR_WRITE and
      registeredVariables[count]->getDataPoolId() != PoolVariableIF::NO_PARAMETER) {
    if (protectEveryReadCommitCall) {
      result =
          registeredVariables[count]->read(timeoutTypeForSingleVars, mutexTimeoutForSingleVars);
    } else {
      /* The readWithoutLock function is protected, so we use the attorney here */
      result = ReadCommitIFAttorney::readWithoutLock(registeredVariables[count]);
    }

    if (result != returnvalue::OK) {
      result = INVALID_PARAMETER_DEFINITION;
    }
  }
  return result;
}

ReturnValue_t PoolDataSetBase::commit(MutexIF::TimeoutType timeoutType, uint32_t lockTimeout) {
  if (state == States::STATE_SET_WAS_READ) {
    handleAlreadyReadDatasetCommit(timeoutType, lockTimeout);
    return returnvalue::OK;
  } else {
    return handleUnreadDatasetCommit(timeoutType, lockTimeout);
  }
}

void PoolDataSetBase::handleAlreadyReadDatasetCommit(MutexIF::TimeoutType timeoutType,
                                                     uint32_t lockTimeout) {
  lockDataPool(timeoutType, lockTimeout);
  for (uint16_t count = 0; count < fillCount; count++) {
    if ((registeredVariables[count]->getReadWriteMode() != PoolVariableIF::VAR_READ) and
        (registeredVariables[count]->getDataPoolId() != PoolVariableIF::NO_PARAMETER)) {
      if (protectEveryReadCommitCall) {
        registeredVariables[count]->commit(timeoutTypeForSingleVars, mutexTimeoutForSingleVars);
      } else {
        /* The commitWithoutLock function is protected, so we use the attorney here */
        ReadCommitIFAttorney::commitWithoutLock(registeredVariables[count]);
      }
    }
  }
  state = States::STATE_SET_UNINITIALISED;
  unlockDataPool();
}

ReturnValue_t PoolDataSetBase::handleUnreadDatasetCommit(MutexIF::TimeoutType timeoutType,
                                                         uint32_t lockTimeout) {
  ReturnValue_t result = returnvalue::OK;
  lockDataPool(timeoutType, lockTimeout);
  for (uint16_t count = 0; count < fillCount; count++) {
    if ((registeredVariables[count]->getReadWriteMode() == PoolVariableIF::VAR_WRITE) and
        (registeredVariables[count]->getDataPoolId() != PoolVariableIF::NO_PARAMETER)) {
      if (protectEveryReadCommitCall) {
        result =
            registeredVariables[count]->commit(timeoutTypeForSingleVars, mutexTimeoutForSingleVars);
      } else {
        /* The commitWithoutLock function is protected, so we use the attorney here */
        ReadCommitIFAttorney::commitWithoutLock(registeredVariables[count]);
      }

    } else if (registeredVariables[count]->getDataPoolId() != PoolVariableIF::NO_PARAMETER) {
      if (result != COMMITING_WITHOUT_READING) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "DataSet::commit(): commit-without-read call made "
                      "with non write-only variable."
                   << std::endl;
#endif
        result = COMMITING_WITHOUT_READING;
      }
    }
  }
  state = States::STATE_SET_UNINITIALISED;
  unlockDataPool();
  return result;
}

ReturnValue_t PoolDataSetBase::lockDataPool(MutexIF::TimeoutType timeoutType,
                                            uint32_t lockTimeout) {
  return returnvalue::OK;
}

ReturnValue_t PoolDataSetBase::unlockDataPool() { return returnvalue::OK; }

ReturnValue_t PoolDataSetBase::serialize(uint8_t** buffer, size_t* size, const size_t maxSize,
                                         SerializeIF::Endianness streamEndianness) const {
  ReturnValue_t result = returnvalue::FAILED;
  for (uint16_t count = 0; count < fillCount; count++) {
    result = registeredVariables[count]->serialize(buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
  }
  return result;
}

ReturnValue_t PoolDataSetBase::deSerialize(const uint8_t** buffer, size_t* size,
                                           SerializeIF::Endianness streamEndianness) {
  ReturnValue_t result = returnvalue::FAILED;
  for (uint16_t count = 0; count < fillCount; count++) {
    result = registeredVariables[count]->deSerialize(buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
  }
  return result;
}

size_t PoolDataSetBase::getSerializedSize() const {
  uint32_t size = 0;
  for (uint16_t count = 0; count < fillCount; count++) {
    size += registeredVariables[count]->getSerializedSize();
  }
  return size;
}

void PoolDataSetBase::setContainer(PoolVariableIF** variablesContainer) {
  this->registeredVariables = variablesContainer;
}

PoolVariableIF** PoolDataSetBase::getContainer() const { return registeredVariables; }

void PoolDataSetBase::setReadCommitProtectionBehaviour(bool protectEveryReadCommit,
                                                       MutexIF::TimeoutType timeoutType,
                                                       uint32_t mutexTimeout) {
  this->protectEveryReadCommitCall = protectEveryReadCommit;
  this->timeoutTypeForSingleVars = timeoutType;
  this->mutexTimeoutForSingleVars = mutexTimeout;
}
