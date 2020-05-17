#include <framework/datapoolglob/GlobalDataPool.h>
#include <framework/datapoolglob/GlobalDataSet.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

GlobDataSet::GlobDataSet(): DataSetBase() {}

// Don't do anything with your variables, they are dead already!
// (Destructor is already called)
GlobDataSet::~GlobDataSet() {}

ReturnValue_t GlobDataSet::commit(bool valid) {
	setEntriesValid(valid);
	setSetValid(valid);
	return commit();
}

ReturnValue_t GlobDataSet::commit() {
	return DataSetBase::commit();
}

ReturnValue_t GlobDataSet::unlockDataPool() {
	return glob::dataPool.freeDataPoolLock();
}

ReturnValue_t GlobDataSet::lockDataPool() {
	return glob::dataPool.lockDataPool();
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


