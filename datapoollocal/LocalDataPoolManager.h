#ifndef FRAMEWORK_HK_HOUSEKEEPINGHELPER_H_
#define FRAMEWORK_HK_HOUSEKEEPINGHELPER_H_
#include <framework/datapool/DataSetIF.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/ipc/MutexIF.h>

#include <framework/housekeeping/HousekeepingMessage.h>
#include <framework/datapool/PoolEntry.h>
#include <framework/datapoollocal/OwnsLocalDataPoolIF.h>
#include <framework/ipc/CommandMessage.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/ipc/MutexHelper.h>

#include <map>

/**
 * @brief 	This class is the managing instance for local data pool.
 * @details
 * The actual data pool structure is a member of this class. Any class which
 * has a local data pool shall have this class as a member and implement
 * the OwnsLocalDataPoolIF.
 *
 * Users of the data pool use the helper classes LocalDataSet,
 * LocalPoolVariable and LocalPoolVector to access pool entries in
 * a thread-safe and efficient way.
 *
 * The local data pools employ a blackboard logic: Only the most recent
 * value is stored. The helper classes offer a read() and commit() interface
 * through the PoolVariableIF which is used to read and update values.
 * Each pool entry has a valid state too.
 *
 */
class LocalDataPoolManager {
	template<typename T>
	friend class LocalPoolVar;
	template<typename T, uint16_t vecSize>
	friend class LocalPoolVector;
	friend class LocalDataSet;
public:
	static constexpr float MINIMAL_SAMPLING_FREQUENCY = 0.2;

	LocalDataPoolManager(OwnsLocalDataPoolIF* owner);
	virtual~ LocalDataPoolManager();


	// propably will just call respective local data set functions.
	void generateHousekeepingPacket(sid_t sid);
	ReturnValue_t handleHousekeepingMessage(CommandMessage* message);

	/**
	 * This function is used to fill the local data pool map with pool
	 * entries. It should only be called once by the pool owner.
	 * @param localDataPoolMap
	 * @return
	 */
	ReturnValue_t initializeHousekeepingPoolEntriesOnce();

	void setHkPacketQueue(MessageQueueIF* msgQueue);
	const OwnsLocalDataPoolIF* getOwner() const;

	ReturnValue_t printPoolEntry(lp_id_t localPoolId);

private:
	//! This is the map holding the actual data. Should only be initialized
	//! once !
	bool mapInitialized = false;
	LocalDataPool localDpMap;

	//! Every housekeeping data manager has a mutex to protect access
	//! to it's data pool.
	MutexIF * mutex = nullptr;

	//! The class which actually owns the manager (and its datapool).
	OwnsLocalDataPoolIF* owner = nullptr;

	//! Used for replies.
	//! (maybe we dont need this, the sender can be retrieved from command
	//! message..)
	MessageQueueIF* hkReplyQueue = nullptr;
	//! Used for HK packets, which are generated without requests.
	//! Maybe this will just be the TM funnel.
	MessageQueueIF* hkPacketQueue = nullptr;

	/**
	 * Get the pointer to the mutex. Can be used to lock the data pool
	 * eternally. Use with care and don't forget to unlock locked mutexes!
	 * For now, only friend classes can accss this function.
	 * @return
	 */
	MutexIF* getMutexHandle();

	/**
	 * Read a variable by supplying its local pool ID and assign the pool
	 * entry to the supplied PoolEntry pointer. The type of the pool entry
	 * is deduced automatically. This call is not thread-safe!
	 * For now, only friend classes like LocalPoolVar may access this
	 * function.
	 * @tparam T Type of the pool entry
	 * @param localPoolId Pool ID of the variable to read
	 * @param poolVar [out] Corresponding pool entry will be assigned to the
	 * 						supplied pointer.
	 * @return
	 */
	template <class T> ReturnValue_t fetchPoolEntry(lp_id_t localPoolId,
			PoolEntry<T> **poolEntry);

	void setMinimalSamplingFrequency(float frequencySeconds);
};


template<class T> inline
ReturnValue_t LocalDataPoolManager::fetchPoolEntry(lp_id_t localPoolId,
		PoolEntry<T> **poolEntry) {
	auto poolIter = localDpMap.find(localPoolId);
	if (poolIter == localDpMap.end()) {
		sif::debug << "HousekeepingManager::fechPoolEntry:"
				" Pool entry not found." << std::endl;
		return OwnsLocalDataPoolIF::POOL_ENTRY_NOT_FOUND;
	}

	*poolEntry = dynamic_cast< PoolEntry<T>* >(poolIter->second);
	if(*poolEntry == nullptr) {
		sif::debug << "HousekeepingManager::fetchPoolEntry:"
				" Pool entry not found." << std::endl;
		return OwnsLocalDataPoolIF::POOL_ENTRY_TYPE_CONFLICT;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


#endif /* FRAMEWORK_HK_HOUSEKEEPINGHELPER_H_ */
