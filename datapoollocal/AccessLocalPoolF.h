#ifndef FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_
#define FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_

#include <fsfw/datapool/PoolEntry.h>
#include <fsfw/datapoollocal/locPoolDefinitions.h>

template <typename T>
class AccessLocalPoolIF {
public:
	virtual~ AccessLocalPoolIF() {};

	virtual ReturnValue_t fetchPoolEntry(lp_id_t localPoolId, PoolEntry<T> **poolEntry) = 0;
};



#endif /* FSFW_DATAPOOLLOCAL_ACCESSLOCALPOOLF_H_ */
