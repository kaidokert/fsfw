#ifndef FSFW_DATAPOOL_READCOMMITIF_H_
#define FSFW_DATAPOOL_READCOMMITIF_H_

#include <fsfw/returnvalues/HasReturnvaluesIF.h>

/**
 * @brief 	Common interface for all software objects which employ read-commit
 * 			semantics.
 */
class ReadCommitIF {
public:
	virtual ~ReadCommitIF() {}
	virtual ReturnValue_t read(uint32_t mutexTimeout) = 0;
	virtual ReturnValue_t commit(uint32_t mutexTimeout) = 0;

protected:

	//! Optional and protected because this is interesting for classes grouping
	//! members with commit and read semantics where the lock is only necessary
	//! once.
	virtual ReturnValue_t readWithoutLock()  {
		return read(20);
	}

	virtual ReturnValue_t commitWithoutLock() {
		return commit(20);
	}
};


#endif /* FSFW_DATAPOOL_READCOMMITIF_H_ */
