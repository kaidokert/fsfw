#include "DataSet.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

DataSet::DataSet() :
		fill_count(0), state(DATA_SET_UNINITIALISED) {
	for (unsigned count = 0; count < DATA_SET_MAX_SIZE; count++) {
		registeredVariables[count] = NULL;
	}
}

DataSet::~DataSet() {
	//Don't do anything with your variables, they are dead already! (Destructor is already called)
}

ReturnValue_t DataSet::read() {
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
		freeDataPoolLock();
	} else {
		sif::error << "DataSet::read(): Call made in wrong position." << std::endl;
		result = SET_WAS_ALREADY_READ;
	}
	return result;
}

ReturnValue_t DataSet::commit(uint8_t valid) {
	setValid(valid);
	return commit();
}

ReturnValue_t DataSet::commit() {
	if (state == DATA_SET_WAS_READ) {
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
		freeDataPoolLock();
		return RETURN_OK;
	} else {
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
					sif::error <<
					        "DataSet::commit(): commit-without-read "
					        "call made with non write-only variable." << std::endl;
					result = COMMITING_WITHOUT_READING;
				}
			}
		}
		state = DATA_SET_UNINITIALISED;
		freeDataPoolLock();
		return result;
	}

}

void DataSet::registerVariable(PoolVariableIF* variable) {
	if (state == DATA_SET_UNINITIALISED) {
		if (variable != NULL) {
			if (fill_count < DATA_SET_MAX_SIZE) {
				registeredVariables[fill_count] = variable;
				fill_count++;
				return;
			}
		}
	}
	sif::error
			<< "DataSet::registerVariable: failed. Either NULL, or set is full, or call made in wrong position."
			<< std::endl;
	return;
}

uint8_t DataSet::freeDataPoolLock() {
	return ::dataPool.freeDataPoolLock();
}

uint8_t DataSet::lockDataPool() {
	return ::dataPool.lockDataPool();
}

ReturnValue_t DataSet::serialize(uint8_t** buffer, size_t* size,
		size_t maxSize, Endianness streamEndianness) const {
	ReturnValue_t result = RETURN_FAILED;
	for (uint16_t count = 0; count < fill_count; count++) {
		result = registeredVariables[count]->serialize(buffer, size, maxSize,
				streamEndianness);
		if (result != RETURN_OK) {
			return result;
		}
	}
	return result;
}

size_t DataSet::getSerializedSize() const {
	size_t size = 0;
	for (uint16_t count = 0; count < fill_count; count++) {
		size += registeredVariables[count]->getSerializedSize();
	}
	return size;
}

void DataSet::setValid(uint8_t valid) {
	for (uint16_t count = 0; count < fill_count; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				!= PoolVariableIF::VAR_READ) {
			registeredVariables[count]->setValid(valid);
		}
	}
}

ReturnValue_t DataSet::deSerialize(const uint8_t** buffer, size_t* size,
		Endianness streamEndianness) {
	ReturnValue_t result = RETURN_FAILED;
	for (uint16_t count = 0; count < fill_count; count++) {
		result = registeredVariables[count]->deSerialize(buffer, size,
				streamEndianness);
		if (result != RETURN_OK) {
			return result;
		}
	}
	return result;
}
