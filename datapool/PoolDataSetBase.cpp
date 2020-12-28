#include "PoolDataSetBase.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

PoolDataSetBase::PoolDataSetBase(PoolVariableIF** registeredVariablesArray,
        const size_t maxFillCount):
        registeredVariables(registeredVariablesArray),
        maxFillCount(maxFillCount) {
}

PoolDataSetBase::~PoolDataSetBase() {}


ReturnValue_t PoolDataSetBase::registerVariable(
		PoolVariableIF *variable) {
	if (state != States::STATE_SET_UNINITIALISED) {
		sif::error << "DataSet::registerVariable: "
				"Call made in wrong position." << std::endl;
		return DataSetIF::DATA_SET_UNINITIALISED;
	}
	if (variable == nullptr) {
		sif::error << "DataSet::registerVariable: "
				"Pool variable is nullptr." << std::endl;
		return DataSetIF::POOL_VAR_NULL;
	}
	if (fillCount >= maxFillCount) {
		sif::error << "DataSet::registerVariable: "
				"DataSet is full." << std::endl;
		return DataSetIF::DATA_SET_FULL;
	}
	registeredVariables[fillCount] = variable;
	fillCount++;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PoolDataSetBase::read(uint32_t lockTimeout) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	ReturnValue_t error = result;
	if (state == States::STATE_SET_UNINITIALISED) {
		lockDataPool(lockTimeout);
		for (uint16_t count = 0; count < fillCount; count++) {
			result = readVariable(count);
			if(result != RETURN_OK) {
				error = result;
			}
		}
		state = States::STATE_SET_WAS_READ;
		unlockDataPool();
	}
	else {
		sif::error << "DataSet::read(): "
				"Call made in wrong position. Don't forget to commit"
				" member datasets!" << std::endl;
		result = SET_WAS_ALREADY_READ;
	}

	if(error != HasReturnvaluesIF::RETURN_OK) {
		result = error;
	}
	return result;
}

uint16_t PoolDataSetBase::getFillCount() const {
    return fillCount;
}

ReturnValue_t PoolDataSetBase::readVariable(uint16_t count) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	if(registeredVariables[count] == nullptr) {
	    // configuration error.
	    return HasReturnvaluesIF::RETURN_FAILED;
	}

	// These checks are often performed by the respective
	// variable implementation too, but I guess a double check does not hurt.
	if (registeredVariables[count]->getReadWriteMode() !=
				PoolVariableIF::VAR_WRITE and
		registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER)
	{
		if(protectEveryReadCommitCall) {
			result = registeredVariables[count]->read(mutexTimeout);
		}
		else {
			result = registeredVariables[count]->readWithoutLock();
		}

		if(result != HasReturnvaluesIF::RETURN_OK) {
			result = INVALID_PARAMETER_DEFINITION;
		}
	}
	return result;
}

ReturnValue_t PoolDataSetBase::commit(uint32_t lockTimeout) {
	if (state == States::STATE_SET_WAS_READ) {
		handleAlreadyReadDatasetCommit(lockTimeout);
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return handleUnreadDatasetCommit(lockTimeout);
	}
}

void PoolDataSetBase::handleAlreadyReadDatasetCommit(uint32_t lockTimeout) {
	lockDataPool(lockTimeout);
	for (uint16_t count = 0; count < fillCount; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				!= PoolVariableIF::VAR_READ
				&& registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			if(protectEveryReadCommitCall) {
				registeredVariables[count]->commit(mutexTimeout);
			}
			else {
				registeredVariables[count]->commitWithoutLock();
			}
		}
	}
	state = States::STATE_SET_UNINITIALISED;
	unlockDataPool();
}

ReturnValue_t PoolDataSetBase::handleUnreadDatasetCommit(uint32_t lockTimeout) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	lockDataPool(lockTimeout);
	for (uint16_t count = 0; count < fillCount; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				== PoolVariableIF::VAR_WRITE
				&& registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			if(protectEveryReadCommitCall) {
				result = registeredVariables[count]->commit(mutexTimeout);
			}
			else {
				result = registeredVariables[count]->commitWithoutLock();
			}

		} else if (registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			if (result != COMMITING_WITHOUT_READING) {
				sif::error << "DataSet::commit(): commit-without-read call made "
						"with non write-only variable." << std::endl;
				result = COMMITING_WITHOUT_READING;
			}
		}
	}
	state = States::STATE_SET_UNINITIALISED;
	unlockDataPool();
	return result;
}


ReturnValue_t PoolDataSetBase::lockDataPool(uint32_t timeoutMs) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PoolDataSetBase::unlockDataPool() {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PoolDataSetBase::serialize(uint8_t** buffer, size_t* size,
		const size_t maxSize, SerializeIF::Endianness streamEndianness) const {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	for (uint16_t count = 0; count < fillCount; count++) {
		result = registeredVariables[count]->serialize(buffer, size, maxSize,
		        streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}
	return result;
}

ReturnValue_t PoolDataSetBase::deSerialize(const uint8_t** buffer, size_t* size,
        SerializeIF::Endianness streamEndianness) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	for (uint16_t count = 0; count < fillCount; count++) {
		result = registeredVariables[count]->deSerialize(buffer, size,
				streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK) {
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

void PoolDataSetBase::setContainer(PoolVariableIF **variablesContainer) {
    this->registeredVariables = variablesContainer;
}

void PoolDataSetBase::setReadCommitProtectionBehaviour(
		bool protectEveryReadCommit, uint32_t mutexTimeout) {
	this->protectEveryReadCommitCall = protectEveryReadCommit;
	this->mutexTimeout = mutexTimeout;
}
