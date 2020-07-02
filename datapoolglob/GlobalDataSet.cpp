#include <framework/datapoolglob/GlobalDataPool.h>
#include <framework/datapoolglob/GlobalDataSet.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

GlobDataSet::GlobDataSet(): DataSetBase(
        reinterpret_cast<PoolVariableIF**>(&registeredVariables),
        DATA_SET_MAX_SIZE) {}

// Don't do anything with your variables, they are dead already!
// (Destructor is already called)
GlobDataSet::~GlobDataSet() {}

ReturnValue_t GlobDataSet::commit(bool valid, uint32_t lockTimeout) {
	setEntriesValid(valid);
	setSetValid(valid);
	return commit(lockTimeout);
}

ReturnValue_t GlobDataSet::commit(uint32_t lockTimeout) {
	return DataSetBase::commit(lockTimeout);
}

ReturnValue_t GlobDataSet::unlockDataPool() {
	return glob::dataPool.unlockDataPool();
}

ReturnValue_t GlobDataSet::lockDataPool(uint32_t timeoutMs) {
	return glob::dataPool.lockDataPool(timeoutMs);
}

void GlobDataSet::setEntriesValid(bool valid) {
	for (uint16_t count = 0; count < fillCount; count++) {
		if (registeredVariables[count]->getReadWriteMode()
				!= PoolVariableIF::VAR_READ) {
			registeredVariables[count]->setValid(valid);
		}
	}
}

void GlobDataSet::setSetValid(bool valid) {
	this->valid = valid;
}


