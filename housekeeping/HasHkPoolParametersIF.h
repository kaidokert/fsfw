#ifndef FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_
#define FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_
#include <framework/datapool/PoolEntryIF.h>
#include <framework/ipc/MessageQueueSenderIF.h>
#include <map>

class HousekeepingManager;
/**
 * @brief	Type definition for local pool entries.
 */
using lp_id_t = uint32_t;
using LocalDataPoolMap =  std::map<lp_id_t, PoolEntryIF*>;
using LocalDataPoolMapIter = LocalDataPoolMap::iterator;

/**
 * @brief
 */
class HasHkPoolParametersIF {
public:
	virtual~ HasHkPoolParametersIF() {};

	virtual MessageQueueId_t getCommandQueue() const = 0;
	virtual ReturnValue_t initializeHousekeepingPoolEntries(
			LocalDataPoolMap& localDataPoolMap) = 0;
	virtual float setMinimalHkSamplingFrequency() = 0;
	virtual HousekeepingManager* getHkManagerHandle() = 0;
};

#endif /* FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_ */
