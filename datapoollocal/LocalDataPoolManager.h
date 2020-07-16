#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_

#include <framework/datapool/DataSetIF.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/ipc/MutexIF.h>

#include <framework/housekeeping/HousekeepingMessage.h>
#include <framework/datapool/PoolEntry.h>
#include <framework/datapoollocal/HasLocalDataPoolIF.h>
#include <framework/ipc/CommandMessage.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/ipc/MutexHelper.h>

#include <map>

class LocalDataSetBase;

/**
 * @brief 	This class is the managing instance for local data pool.
 * @details
 * The actual data pool structure is a member of this class. Any class which
 * has a local data pool shall have this class as a member and implement
 * the HasLocalDataPoolIF.
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
	friend class LocalDataSetBase;
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::HOUSEKEEPING_MANAGER;

    static constexpr ReturnValue_t POOL_ENTRY_NOT_FOUND = MAKE_RETURN_CODE(0x0);
    static constexpr ReturnValue_t POOL_ENTRY_TYPE_CONFLICT = MAKE_RETURN_CODE(0x1);

    static constexpr ReturnValue_t QUEUE_OR_DESTINATION_NOT_SET = MAKE_RETURN_CODE(0x2);
    //static constexpr ReturnValue_t SET_NOT_FOUND = MAKE_RETURN_CODE(0x3);

    /**
     * This constructor is used by a class which wants to implement
     * a personal local data pool. The queueToUse can be supplied if it
     * is already known.
     *
     * initialize() has to be called in any case before using the object!
     * @param owner
     * @param queueToUse
     * @param appendValidityBuffer
     */
	LocalDataPoolManager(HasLocalDataPoolIF* owner, MessageQueueIF* queueToUse,
	        bool appendValidityBuffer = true);
	virtual~ LocalDataPoolManager();

	/**
	 * Initializes the map by calling the map initialization function of the
	 * owner and assigns the queue to use.
	 * @param queueToUse
	 * @return
	 */
	ReturnValue_t initialize(MessageQueueIF* queueToUse,
	        object_id_t hkDestination);

	/**
	 * This should be called in the periodic handler of the owner.
	 * It performs all the periodic functionalities of the data pool manager.
	 * @return
	 */
	ReturnValue_t performHkOperation();
	/**
	 * This function is used to set the default HK packet destination.
	 * This destination will usually only be set once.
	 * @param hkDestination
	 */
	void setHkPacketDestination(MessageQueueId_t hkDestination);

	/**
	 * Generate a housekeeping packet with a given SID.
	 * @param sid
	 * @return
	 */
	ReturnValue_t generateHousekeepingPacket(sid_t sid, MessageQueueId_t sendTo
	        = MessageQueueIF::NO_QUEUE);
	ReturnValue_t generateSetStructurePacket(sid_t sid);

	ReturnValue_t handleHousekeepingMessage(CommandMessage* message);

	/**
	 * This function is used to fill the local data pool map with pool
	 * entries. It should only be called once by the pool owner.
	 * @param localDataPoolMap
	 * @return
	 */
	ReturnValue_t initializeHousekeepingPoolEntriesOnce();

	const HasLocalDataPoolIF* getOwner() const;

	ReturnValue_t printPoolEntry(lp_id_t localPoolId);

    /**
     * Different types of housekeeping reporting are possible.
     *  1. PERIODIC: HK packets are generated in fixed intervals and sent to
     *     destination. Fromat will be raw.
     *  2. UPDATED: Notification will be sent out if HK data has changed.
     *     Question: Send Raw data directly or just the message?
     *  3. REQUESTED: HK packets are only generated if explicitely requested.
     *     Propably not necessary, just use multiple local data sets or
     *     shared datasets.
     *
     *  TODO: This is a big architecture question. Use raw data or shared
     *  datasets? dumb thing about shared datasets: It propably would be better
     *  if each task who uses the data has their own copy of the pool
     *  variables. Then the LPIDs should be hardcoded in the dataset implementations
     *  so the users don't have to worry about it anymore.
     *
     *  Notifications should also be possible for single variables instead of
     *  full dataset updates.
     */
    enum class ReportingType: uint8_t {
        // Periodic generation of HK packets.
        PERIODIC,
        // Notification will be sent out as message.
        // Data is accessed via shared data set or multiple local data sets.
        ON_UPDATE,
        // actually, requested is propably unnecessary. If another component
        // needs data on request, they can just use  the new SharedDataSet
        // which is thread-safe to also have full access to the interface..
        REQUESTED
    };

    /* Copying forbidden */
    LocalDataPoolManager(const LocalDataPoolManager &) = delete;
    LocalDataPoolManager operator=(const LocalDataPoolManager&) = delete;

private:
    LocalDataPool localPoolMap;
    /** Every housekeeping data manager has a mutex to protect access
     * to it's data pool. */
    MutexIF* mutex = nullptr;
    /** The class which actually owns the manager (and its datapool). */
    HasLocalDataPoolIF* owner = nullptr;

    /**
     * The data pool manager will keep an internal map of HK receivers.
     */
    struct HkReceiver {
        LocalDataSetBase* dataSet = nullptr;
        lp_id_t localPoolId = HasLocalDataPoolIF::NO_POOL_ID;
        MessageQueueId_t destinationQueue = MessageQueueIF::NO_QUEUE;
        ReportingType reportingType = ReportingType::PERIODIC;
        bool reportingStatus = true;
        /** Different members of this union will be used depending on reporting
         * type */
        union hkParameter {
            /** This parameter will be used for the PERIODIC type */
            dur_seconds_t collectionInterval = 0;
            /** This parameter will be used for the ON_UPDATE type */
            bool hkDataChanged;
        };
    };

    /** Using a multimap as the same object might request multiple datasets */
    using HkReceiversMap = std::multimap<object_id_t, struct HkReceiver>;

    HkReceiversMap hkReceiversMap;

    /** This is the map holding the actual data. Should only be initialized
     * once ! */
    bool mapInitialized = false;
    /** This specifies whether a validity buffer is appended at the end
     * of generated housekeeping packets. */
    bool appendValidityBuffer = true;

	/**
	 * @brief Queue used for communication, for example commands.
	 * Is also used to send messages. Can be set either in the constructor
     * or in the initialize() function.
	 */
	MessageQueueIF* hkQueue = nullptr;

	/**
	 * HK replies will always be a reply to the commander, but HK packet
	 * can be sent to another destination by specifying this message queue
	 * ID, for example to a dedicated housekeeping service implementation.
	 */
	MessageQueueId_t hkDestination = MessageQueueIF::NO_QUEUE;

	/** Global IPC store is used to store all packets. */
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
	        LocalDataSetBase* dataSet);
};


template<class T> inline
ReturnValue_t LocalDataPoolManager::fetchPoolEntry(lp_id_t localPoolId,
		PoolEntry<T> **poolEntry) {
	auto poolIter = localPoolMap.find(localPoolId);
	if (poolIter == localPoolMap.end()) {
		sif::warning << "HousekeepingManager::fechPoolEntry: Pool entry "
		        "not found." << std::endl;
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


#endif /* FRAMEWORK_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_ */
