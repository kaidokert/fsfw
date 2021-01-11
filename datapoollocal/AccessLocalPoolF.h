#ifndef FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_
#define FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_

#include <fsfw/datapoollocal/locPoolDefinitions.h>
#include <fsfw/datapool/PoolEntry.h>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/ipc/MutexIF.h>

class AccessLocalPoolIF {
public:
	virtual ~AccessLocalPoolIF() {};

	virtual ReturnValue_t retrieveLocalPoolMutex(MutexIF* mutex) = 0;
	virtual object_id_t getCreatorObjectId() const = 0;

protected:

	//virtual LocalDataPoolManager* getHkManagerHandle() = 0;
};

//template <typename T>
//class AccessLocalPoolTypedIF {
//public:
//	virtual ~AccessLocalPoolTypedIF() {};
//
//	virtual ReturnValue_t fetchPoolEntry(lp_id_t localPoolId, PoolEntry<T> **poolEntry) = 0;
//};



#endif /* FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_ */
