#include <framework/datapoollocal/LocalDataSet.h>

LocalDataSet::LocalDataSet():
		fill_count(0), state(DATA_SET_UNINITIALISED)
{
	for (unsigned count = 0; count < DATA_SET_MAX_SIZE; count++) {
		registeredVariables[count] = nullptr;
	}
}

// who has the responsibility to lock the mutex? the local pool variable
// has access to the HK manager and could call its mutex lock function.
ReturnValue_t LocalDataSet::registerVariable(
		PoolVariableIF *variable) {
	return RETURN_OK;
}

LocalDataSet::~LocalDataSet() {
}

ReturnValue_t LocalDataSet::read() {
	return RETURN_OK;
}

ReturnValue_t LocalDataSet::commit(void) {
	return RETURN_OK;
}

ReturnValue_t LocalDataSet::commit(bool valid) {
	return RETURN_OK;
}

void LocalDataSet::setSetValid(bool valid) {
}

void LocalDataSet::setEntriesValid(bool valid) {
}

ReturnValue_t LocalDataSet::serialize(uint8_t **buffer,
		size_t *size, const size_t max_size, bool bigEndian) const {
	return RETURN_OK;
}

size_t LocalDataSet::getSerializedSize() const {
	return 0;
}

ReturnValue_t LocalDataSet::deSerialize(const uint8_t **buffer,
		size_t *size, bool bigEndian) {
	return RETURN_OK;
}

ReturnValue_t LocalDataSet::lockDataPool() {
	return RETURN_OK;
}

ReturnValue_t LocalDataSet::unlockDataPool() {
	return RETURN_OK;
}

void LocalDataSet::handleAlreadyReadDatasetCommit() {
}

ReturnValue_t LocalDataSet::handleUnreadDatasetCommit() {
	return RETURN_OK;
}
