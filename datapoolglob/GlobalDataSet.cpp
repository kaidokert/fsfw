#include <framework/datapoolglob/GlobalDataSet.h>
#include <framework/datapoolglob/GlobalDataPool.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

GlobDataSet::GlobDataSet() :
		fill_count(0), state(DATA_SET_UNINITIALISED) {
	for (unsigned count = 0; count < DATA_SET_MAX_SIZE; count++) {
		registeredVariables[count] = nullptr;
	}
}

GlobDataSet::~GlobDataSet() {
	//Don't do anything with your variables, they are dead already!
	// (Destructor is already called)
}

ReturnValue_t GlobDataSet::registerVariable(PoolVariableIF* variable) {
	if (state != DATA_SET_UNINITIALISED) {
		sif::error << "DataSet::registerVariable: Call made in wrong position." << std::endl;
		return DATA_SET_UNINITIALISED;
	}
	if (variable == nullptr) {
		sif::error << "DataSet::registerVariable: Pool variable is nullptr." << std::endl;
		return POOL_VAR_NULL;
	}
	if (fill_count >= DATA_SET_MAX_SIZE) {
		sif::error << "DataSet::registerVariable: DataSet is full." << std::endl;
		return DATA_SET_FULL;
	}
	registeredVariables[fill_count] = variable;
	fill_count++;
	return RETURN_OK;
}

ReturnValue_t GlobDataSet::read() {
	ReturnValue_t result = RETURN_OK;
	if (state == DATA_SET_UNINITIALISED) {
		lockDataPool();
		for (uint16_t count = 0; count < fill_count; count++) {
			if (registeredVariables[count]->getReadWriteMode()
					!= PoolVariableIF::VAR_WRITE
					&& registeredVariables[count]->getDataPoolId()
							!= PoolVariableIF::NO_PARAMETER) {
				ReturnValue_t status = registeredVariables[count]->read();
				if (status != RETURN_OK) {
					result = INVALID_PARAMETER_DEFINITION;
					break;
				}
			}
		}
		state = DATA_SET_WAS_READ;
		unlockDataPool();
	} else {
		sif::error << "DataSet::read(): Call made in wrong position." << std::endl;
		result = SET_WAS_ALREADY_READ;
	}
	return result;
}

ReturnValue_t GlobDataSet::commit(bool valid) {
	setEntriesValid(valid);
	setSetValid(valid);
	return commit();
}

ReturnValue_t GlobDataSet::commit() {
	if (state == DATA_SET_WAS_READ) {
		handleAlreadyReadDatasetCommit();
		return RETURN_OK;
	}
	else {
		return handleUnreadDatasetCommit();
	}
}

void GlobDataSet::handleAlreadyReadDatasetCommit() {
	lockDataPool();
	for (uint16_t count = 0; count < fill_count; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				!= PoolVariableIF::VAR_READ
				&& registeredVariables[count]->getDataPoolId()
				!= PoolVariableIF::NO_PARAMETER) {
			registeredVariables[count]->commit();
		}
	}
	state = DATA_SET_UNINITIALISED;
	unlockDataPool();
}

ReturnValue_t GlobDataSet::handleUnreadDatasetCommit() {
	ReturnValue_t result = RETURN_OK;
	lockDataPool();
	for (uint16_t count = 0; count < fill_count; count++) {
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
	state = DATA_SET_UNINITIALISED;
	unlockDataPool();
	return result;
}

ReturnValue_t GlobDataSet::unlockDataPool() {
	return ::dataPool.freeDataPoolLock();
}

ReturnValue_t GlobDataSet::lockDataPool() {
	return ::dataPool.lockDataPool();
}

ReturnValue_t GlobDataSet::serialize(uint8_t** buffer, size_t* size,
		const size_t max_size, bool bigEndian) const {
	ReturnValue_t result = RETURN_FAILED;
	for (uint16_t count = 0; count < fill_count; count++) {
		result = registeredVariables[count]->serialize(buffer, size, max_size,
				bigEndian);
		if (result != RETURN_OK) {
			return result;
		}
	}
	return result;
}

size_t GlobDataSet::getSerializedSize() const {
	uint32_t size = 0;
	for (uint16_t count = 0; count < fill_count; count++) {
		size += registeredVariables[count]->getSerializedSize();
	}
	return size;
}

void GlobDataSet::setEntriesValid(bool valid) {
	for (uint16_t count = 0; count < fill_count; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				!= PoolVariableIF::VAR_READ) {
			registeredVariables[count]->setValid(valid);
		}
	}
}

void GlobDataSet::setSetValid(bool valid) {
	this->valid = valid;
}

ReturnValue_t GlobDataSet::deSerialize(const uint8_t** buffer, size_t* size,
		bool bigEndian) {
	ReturnValue_t result = RETURN_FAILED;
	for (uint16_t count = 0; count < fill_count; count++) {
		result = registeredVariables[count]->deSerialize(buffer, size,
				bigEndian);
		if (result != RETURN_OK) {
			return result;
		}
	}
	return result;
}
