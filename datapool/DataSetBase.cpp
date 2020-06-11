#include <framework/datapool/DataSetBase.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

DataSetBase::DataSetBase() {
	for (uint8_t count = 0; count < DATA_SET_MAX_SIZE; count++) {
		registeredVariables[count] = nullptr;
	}
}

DataSetBase::~DataSetBase() {}

ReturnValue_t DataSetBase::registerVariable(
		PoolVariableIF *variable) {
	if (state != States::DATA_SET_UNINITIALISED) {
		sif::error << "DataSet::registerVariable: "
				"Call made in wrong position." << std::endl;
		return DataSetIF::DATA_SET_UNINITIALISED;
	}
	if (variable == nullptr) {
		sif::error << "DataSet::registerVariable: "
				"Pool variable is nullptr." << std::endl;
		return DataSetIF::POOL_VAR_NULL;
	}
	if (fillCount >= DATA_SET_MAX_SIZE) {
		sif::error << "DataSet::registerVariable: "
				"DataSet is full." << std::endl;
		return DataSetIF::DATA_SET_FULL;
	}
	registeredVariables[fillCount] = variable;
	fillCount++;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t DataSetBase::read(uint32_t lockTimeout) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	if (state == States::DATA_SET_UNINITIALISED) {
		lockDataPool(lockTimeout);
		for (uint16_t count = 0; count < fillCount; count++) {
			result = readVariable(count);
			if(result != RETURN_OK) {
				break;
			}
		}
		state = States::DATA_SET_WAS_READ;
		unlockDataPool();
	}
	else {
		sif::error << "DataSet::read(): "
				"Call made in wrong position. Don't forget to commit"
				" member datasets!" << std::endl;
		result = SET_WAS_ALREADY_READ;
	}
	return result;
}

ReturnValue_t DataSetBase::readVariable(uint16_t count) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	// These checks are often performed by the respective
	// variable implementation too, but I guess a double check does not hurt.
	if (registeredVariables[count]->getReadWriteMode() !=
				PoolVariableIF::VAR_WRITE and
		registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER)
	{
		result = registeredVariables[count]->readWithoutLock();
		if(result != HasReturnvaluesIF::RETURN_OK) {
			result = INVALID_PARAMETER_DEFINITION;
		}
	}
	return result;
}

ReturnValue_t DataSetBase::commit(uint32_t lockTimeout) {
	if (state == States::DATA_SET_WAS_READ) {
		handleAlreadyReadDatasetCommit(lockTimeout);
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return handleUnreadDatasetCommit(lockTimeout);
	}
}

void DataSetBase::handleAlreadyReadDatasetCommit(uint32_t lockTimeout) {
	lockDataPool(lockTimeout);
	for (uint16_t count = 0; count < fillCount; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				!= PoolVariableIF::VAR_READ
				&& registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			registeredVariables[count]->commitWithoutLock();
		}
	}
	state = States::DATA_SET_UNINITIALISED;
	unlockDataPool();
}

ReturnValue_t DataSetBase::handleUnreadDatasetCommit(uint32_t lockTimeout) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	lockDataPool(lockTimeout);
	for (uint16_t count = 0; count < fillCount; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				== PoolVariableIF::VAR_WRITE
				&& registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			registeredVariables[count]->commitWithoutLock();
		} else if (registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			if (result != COMMITING_WITHOUT_READING) {
				sif::error << "DataSet::commit(): commit-without-read call made "
						"with non write-only variable." << std::endl;
				result = COMMITING_WITHOUT_READING;
			}
		}
	}
	state = States::DATA_SET_UNINITIALISED;
	unlockDataPool();
	return result;
}


ReturnValue_t DataSetBase::lockDataPool(uint32_t timeoutMs) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t DataSetBase::unlockDataPool() {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t DataSetBase::serialize(uint8_t** buffer, size_t* size,
		const size_t maxSize, bool bigEndian) const {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	for (uint16_t count = 0; count < fillCount; count++) {
		result = registeredVariables[count]->serialize(buffer, size, maxSize,
				bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}
	return result;
}

ReturnValue_t DataSetBase::deSerialize(const uint8_t** buffer, size_t* size,
		bool bigEndian) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	for (uint16_t count = 0; count < fillCount; count++) {
		result = registeredVariables[count]->deSerialize(buffer, size,
				bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}
	return result;
}

size_t DataSetBase::getSerializedSize() const {
	uint32_t size = 0;
	for (uint16_t count = 0; count < fillCount; count++) {
		size += registeredVariables[count]->getSerializedSize();
	}
	return size;
}
