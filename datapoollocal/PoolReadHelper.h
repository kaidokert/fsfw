#ifndef FSFW_DATAPOOLLOCAL_POOLREADHELPER_H_
#define FSFW_DATAPOOLLOCAL_POOLREADHELPER_H_

#include <fsfw/datapoollocal/LocalPoolDataSetBase.h>
#include <FSFWConfig.h>

/**
 * @brief 	Helper class to read data sets or pool variables
 */
class PoolReadHelper {
public:
	PoolReadHelper(ReadCommitIF* readObject, uint32_t mutexTimeout = 20):
			readObject(readObject), mutexTimeout(mutexTimeout) {
		if(readObject != nullptr) {
			readResult = readObject->read(mutexTimeout);
#if FSFW_PRINT_VERBOSITY_LEVEL == 1
#if CPP_OSTREAM_ENABLED == 1
			sif::error << "PoolReadHelper: Read failed!" << std::endl;
#endif
#endif
		}
	}

	ReturnValue_t getReadResult() const {
		return readResult;
	}

	~PoolReadHelper() {
		if(readObject != nullptr) {
			readObject->commit(mutexTimeout);
		}

	}

private:
	ReadCommitIF* readObject = nullptr;
	ReturnValue_t readResult = HasReturnvaluesIF::RETURN_OK;
	uint32_t mutexTimeout = 20;
};



#endif /* FSFW_DATAPOOLLOCAL_POOLREADHELPER_H_ */
