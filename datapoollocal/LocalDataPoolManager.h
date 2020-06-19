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

class LocalDataSet;

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
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::HOUSEKEEPING_MANAGER;

    static constexpr ReturnValue_t POOL_ENTRY_NOT_FOUND = MAKE_RETURN_CODE(0x0);
    static constexpr ReturnValue_t POOL_ENTRY_TYPE_CONFLICT = MAKE_RETURN_CODE(0x1);

    static constexpr ReturnValue_t QUEUE_NOT_SET = MAKE_RETURN_CODE(0x2);
    //static constexpr ReturnValue_t SET_NOT_FOUND = MAKE_RETURN_CODE(0x3);

	LocalDataPoolManager(OwnsLocalDataPoolIF* owner,
	        uint32_t replyQueueDepth = 20, bool appendValidityBuffer = true);
	virtual~ LocalDataPoolManager();

	/* Copying forbidden */
	LocalDataPoolManager(const LocalDataPoolManager &) = delete;
	LocalDataPoolManager operator=(const LocalDataPoolManager&) = delete;

	ReturnValue_t generateHousekeepingPacket(sid_t sid);
	ReturnValue_t handleHousekeepingMessage(HousekeepingMessage& message);

	/**
	 * This function is used to fill the local data pool map with pool
	 * entries. It should only be called once by the pool owner.
	 * @param localDataPoolMap
	 * @return
	 */
	ReturnValue_t initializeHousekeepingPoolEntriesOnce();

	//! Set the queue for HK packets, which are sent unrequested.
	void setHkPacketDestination(MessageQueueId_t destinationQueueId);

	const OwnsLocalDataPoolIF* getOwner() const;

	ReturnValue_t printPoolEntry(lp_id_t localPoolId);

private:
	//! This is the map holding the actual data. Should only be initialized
	//! once !
	bool mapInitialized = false;
	//! This specifies whether a validity buffer is appended at the end
	//! of generated housekeeping packets.
	bool appendValidityBuffer = true;

	LocalDataPool localDpMap;

	//! Every housekeeping data manager has a mutex to protect access
	//! to it's data pool.
	MutexIF * mutex = nullptr;

	//! The class which actually owns the manager (and its datapool).
	OwnsLocalDataPoolIF* owner = nullptr;

	//! Queue used for communication, for example commands.
	//! Is also used to send messages.
	MessageQueueIF* hkQueue = nullptr;

	//! HK replies will always be a reply to the commander, but HK packet
	//! can be sent to another destination by specifying this message queue
	//! ID, for example to a dedicated housekeeping service implementation.
	MessageQueueId_t currentHkPacketDestination = MessageQueueIF::NO_QUEUE;

	//! Global IPC store is used to store all packets.
	StorageManagerIF* ipcStore = nullptr;
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
	ReturnValue_t serializeHkPacketIntoStore(store_address_t* storeId,
	        LocalDataSet* dataSet);
};


template<class T> inline
ReturnValue_t LocalDataPoolManager::fetchPoolEntry(lp_id_t localPoolId,
		PoolEntry<T> **poolEntry) {
	auto poolIter = localDpMap.find(localPoolId);
	if (poolIter == localDpMap.end()) {
		sif::debug << "HousekeepingManager::fechPoolEntry:"
				" Pool entry not found." << std::endl;
		return POOL_ENTRY_NOT_FOUND;
	}

	*poolEntry = dynamic_cast< PoolEntry<T>* >(poolIter->second);
	if(*poolEntry == nullptr) {
		sif::debug << "HousekeepingManager::fetchPoolEntry:"
				" Pool entry not found." << std::endl;
		return POOL_ENTRY_TYPE_CONFLICT;
	}
	return HasReturnvaluesIF::RETURN_OK;
}


#endif /* FRAMEWORK_HK_HOUSEKEEPINGHELPER_H_ */
