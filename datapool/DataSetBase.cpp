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

ReturnValue_t DataSetBase::read() {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	if (state == States::DATA_SET_UNINITIALISED) {
		lockDataPool();
		for (uint16_t count = 0; count < fillCount; count++) {
			if (registeredVariables[count]->getReadWriteMode()
					!= PoolVariableIF::VAR_WRITE
					&& registeredVariables[count]->getDataPoolId()
					!= PoolVariableIF::NO_PARAMETER) {
				ReturnValue_t status = registeredVariables[count]->read();
				if (status !=  HasReturnvaluesIF::RETURN_OK) {
					result = INVALID_PARAMETER_DEFINITION;
					break;
				}
			}
		}
		state = States::DATA_SET_WAS_READ;
		unlockDataPool();
	}
	else {
		sif::error << "DataSet::read(): "
				"Call made in wrong position." << std::endl;
		result = SET_WAS_ALREADY_READ;
	}
	return result;
}

ReturnValue_t DataSetBase::commit() {
	if (state == States::DATA_SET_WAS_READ) {
		handleAlreadyReadDatasetCommit();
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return handleUnreadDatasetCommit();
	}
}

ReturnValue_t DataSetBase::lockDataPool() {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t DataSetBase::unlockDataPool() {
	return HasReturnvaluesIF::RETURN_FAILED;
}

void DataSetBase::handleAlreadyReadDatasetCommit() {
	lockDataPool();
	for (uint16_t count = 0; count < fillCount; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				!= PoolVariableIF::VAR_READ
				&& registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			registeredVariables[count]->commit();
		}
	}
	state = States::DATA_SET_UNINITIALISED;
	unlockDataPool();
}

ReturnValue_t DataSetBase::handleUnreadDatasetCommit() {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	lockDataPool();
	for (uint16_t count = 0; count < fillCount; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				== PoolVariableIF::VAR_WRITE
				&& registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			registeredVariables[count]->commit();
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
