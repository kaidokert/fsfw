#ifndef FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_
#define FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_
#include <framework/datapool/PoolEntryIF.h>
#include <framework/ipc/MessageQueueSenderIF.h>
#include <framework/housekeeping/HousekeepingMessage.h>
#include <map>

class LocalDataPoolManager;
class DataSetIF;
/**
 * @brief	Type definition for local pool entries.
 */
using lp_id_t = uint32_t;
using LocalDataPool =  std::map<lp_id_t, PoolEntryIF*>;
using LocalDataPoolMapIter = LocalDataPool::iterator;

/**
 * @brief 		This interface is implemented by classes which posses a local
 * 				data pool (not the managing class). It defines the relationship
 * 				between the local data pool owner and the LocalDataPoolManager.
 * @details
 * Any class implementing this interface shall also have a LocalDataPoolManager
 * member class which contains the actual pool data structure
 * and exposes the public interface for it.
 * This is required because the pool entries are templates, which makes
 * specifying an interface rather difficult. The local data pool can be
 * accessed by using the LocalPoolVariable, LocalPoolVector or LocalDataSet
 * classes.
 *
 * Architectural Note:
 * This could be circumvented by using a wrapper/accessor function or
 * implementing the templated function in this interface..
 * The first solution sounds better than the second but
 * the LocalPoolVariable classes are templates as well, so this just shifts
 * the problem somewhere else. Interfaces are nice, but the most
 * pragmatic solution I found was to offer the client the full interface
 * of the LocalDataPoolManager.
 */
class OwnsLocalDataPoolIF {
public:
	virtual~ OwnsLocalDataPoolIF() {};

	static constexpr uint8_t INTERFACE_ID = CLASS_ID::LOCAL_POOL_OWNER_IF;

	virtual MessageQueueId_t getCommandQueue() const = 0;
	virtual ReturnValue_t initializeHousekeepingPoolEntries(
			LocalDataPool& localDataPoolMap) = 0;
	//virtual float setMinimalHkSamplingFrequency() = 0;
	virtual LocalDataPoolManager* getHkManagerHandle() = 0;
	virtual DataSetIF* getDataSetHandle(sid_t sid) = 0;
};

#endif /* FRAMEWORK_DATAPOOL_HASHKPOOLPARAMETERSIF_H_ */
