#ifndef FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_
#define FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_
#include <framework/datapool/PoolEntryIF.h>
#include <framework/ipc/MessageQueueSenderIF.h>
#include <framework/housekeeping/HousekeepingMessage.h>
#include <map>

class HousekeepingManager;
class DataSetIF;
/**
 * @brief	Type definition for local pool entries.
 */
using lp_id_t = uint32_t;
using LocalDataPoolMap =  std::map<lp_id_t, PoolEntryIF*>;
using LocalDataPoolMapIter = LocalDataPoolMap::iterator;

/**
 * @brief 		This interface is implemented by classes which posses a local
 * 				data pool (not the managing class)
 * @details
 * Any class implementing this interface shall also have a HousekeepingManager
 * member class which handles the retrieval of the local pool data.
 * This is required because the pool entries are templates, which makes
 * specifying an interface rather difficult.
 *
 * This could be circumvented by using a wrapper/accessor function, but
 * the LocalPoolVariable classes are templates as well, so this just shifts
 * the problem somewhere else. Interfaces are nice, but the most
 * pragmatic solution I found was to offer the client the full interface
 * of the housekeeping manager.
 */
class HasHkPoolParametersIF {
public:
	virtual~ HasHkPoolParametersIF() {};

	static constexpr uint8_t INTERFACE_ID = CLASS_ID::HOUSEKEEPING;
	static constexpr ReturnValue_t POOL_ENTRY_NOT_FOUND = MAKE_RETURN_CODE(0XA0);
	static constexpr ReturnValue_t POOL_ENTRY_TYPE_CONFLICT = MAKE_RETURN_CODE(0xA1);

	virtual MessageQueueId_t getCommandQueue() const = 0;
	virtual ReturnValue_t initializeHousekeepingPoolEntries(
			LocalDataPoolMap& localDataPoolMap) = 0;
	//virtual float setMinimalHkSamplingFrequency() = 0;
	virtual HousekeepingManager* getHkManagerHandle() = 0;
	virtual DataSetIF* getDataSetHandle(sid_t sid) = 0;
};

#endif /* FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_ */
