#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_

#include "HasLocalDataPoolIF.h"

#include "../housekeeping/HousekeepingPacketDownlink.h"
#include "../housekeeping/HousekeepingMessage.h"
#include "../housekeeping/PeriodicHousekeepingHelper.h"
#include "../datapool/DataSetIF.h"
#include "../datapool/PoolEntry.h"
#include "../objectmanager/SystemObjectIF.h"
#include "../ipc/MutexIF.h"
#include "../ipc/CommandMessage.h"
#include "../ipc/MessageQueueIF.h"
#include "../ipc/MutexHelper.h"

#include <map>

namespace Factory {
void setStaticFrameworkObjectIds();
}

class LocalDataSetBase;


/**
 * @brief 		This class is the managing instance for the local data pool.
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
 * @author 		R. Mueller
 */
class LocalDataPoolManager {
	template<typename T>
	friend class LocalPoolVar;
	template<typename T, uint16_t vecSize>
	friend class LocalPoolVector;
	friend class LocalPoolDataSetBase;
	friend void (Factory::setStaticFrameworkObjectIds)();
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::HOUSEKEEPING_MANAGER;

    static constexpr ReturnValue_t POOL_ENTRY_NOT_FOUND = MAKE_RETURN_CODE(0x00);
    static constexpr ReturnValue_t POOL_ENTRY_TYPE_CONFLICT = MAKE_RETURN_CODE(0x01);

    static constexpr ReturnValue_t QUEUE_OR_DESTINATION_NOT_SET = MAKE_RETURN_CODE(0x02);

    static constexpr ReturnValue_t WRONG_HK_PACKET_TYPE = MAKE_RETURN_CODE(0x03);
    static constexpr ReturnValue_t REPORTING_STATUS_UNCHANGED = MAKE_RETURN_CODE(0x04);
    static constexpr ReturnValue_t PERIODIC_HELPER_INVALID = MAKE_RETURN_CODE(0x05);
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
	 * @param nonDiagInvlFactor See #setNonDiagnosticIntervalFactor doc
	 * @return
	 */
	ReturnValue_t initialize(MessageQueueIF* queueToUse);

	ReturnValue_t initializeAfterTaskCreation(uint8_t nonDiagInvlFactor = 5);

	/**
	 * @return
	 */
	ReturnValue_t subscribeForPeriodicPacket(sid_t sid, bool enableReporting,
			float collectionInterval, bool isDiagnostics,
			object_id_t packetDestination = defaultHkDestination);

	/**
	 * Non-Diagnostics packets usually have a lower minimum sampling frequency
	 * than diagnostic packets.
	 * A factor can be specified to determine the minimum sampling frequency
	 * for non-diagnostic packets. The minimum sampling frequency of the
	 * diagnostics packets,which is usually jusst the period of the
	 * performOperation calls, is multiplied with that factor.
	 * @param factor
	 */
	void setNonDiagnosticIntervalFactor(uint8_t nonDiagInvlFactor);

	/**
	 * This should be called in the periodic handler of the owner.
	 * It performs all the periodic functionalities of the data pool manager.
	 * @return
	 */
	ReturnValue_t performHkOperation();

	/**
	 * Generate a housekeeping packet with a given SID.
	 * @param sid
	 * @return
	 */
	ReturnValue_t generateHousekeepingPacket(sid_t sid,
			LocalPoolDataSetBase* dataSet, bool forDownlink,
			MessageQueueId_t destination = MessageQueueIF::NO_QUEUE);

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
     *  1. PERIODIC:
     *     HK packets are generated in fixed intervals and sent to
     *     destination. Fromat will be raw.
     *  2. UPDATE_NOTIFICATION:
     *     Notification will be sent out if HK data has changed.
     *  3. UPDATE_SNAPSHOT:
     *     HK packets are only generated if explicitely requested.
     *     Propably not necessary, just use multiple local data sets or
     *     shared datasets.
     */
    enum class ReportingType: uint8_t {
        //! Periodic generation of HK packets.
        PERIODIC,
		//! Update notification will be sent out as message.
		UPDATE_NOTIFICATION,
        //! Notification will be sent out as message and a snapshot of the
        //! current data will be generated.
        UPDATE_SNAPSHOT,
    };

    /**
     * Different data types are possible in the HK receiver map.
     * For example, updates can be requested for full datasets or
     * for single pool variables. Periodic reporting is only possible for
     * data sets.
     */
    enum class DataType: uint8_t {
    	LOCAL_POOL_VARIABLE,
		DATA_SET
    };

    /* Copying forbidden */
    LocalDataPoolManager(const LocalDataPoolManager &) = delete;
    LocalDataPoolManager operator=(const LocalDataPoolManager&) = delete;

private:
    LocalDataPool localPoolMap;
    //! Every housekeeping data manager has a mutex to protect access
    //! to it's data pool.
    MutexIF* mutex = nullptr;
    /** The class which actually owns the manager (and its datapool). */
    HasLocalDataPoolIF* owner = nullptr;

    uint8_t nonDiagnosticIntervalFactor = 0;

	/** Default receiver for periodic HK packets */
	static object_id_t defaultHkDestination;
	MessageQueueId_t hkDestinationId = MessageQueueIF::NO_QUEUE;

    /** The data pool manager will keep an internal map of HK receivers. */
    struct HkReceiver {
		/** Object ID of receiver */
		object_id_t objectId = objects::NO_OBJECT;

		DataType dataType = DataType::DATA_SET;
        union DataId {
			DataId(): sid() {};
            sid_t sid;
            lp_id_t localPoolId;
        };
        DataId dataId;

        ReportingType reportingType = ReportingType::PERIODIC;
        MessageQueueId_t destinationQueue = MessageQueueIF::NO_QUEUE;
    };

    /** This vector will contain the list of HK receivers. */
    using HkReceivers = std::vector<struct HkReceiver>;

    HkReceivers hkReceiversMap;

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

	ReturnValue_t serializeHkPacketIntoStore(
	        HousekeepingPacketDownlink& hkPacket,
	        store_address_t& storeId, bool forDownlink, size_t* serializedSize);

	void performPeriodicHkGeneration(HkReceiver& hkReceiver);
	ReturnValue_t togglePeriodicGeneration(sid_t sid, bool enable,
			bool isDiagnostics);
	ReturnValue_t changeCollectionInterval(sid_t sid,
			float newCollectionInterval, bool isDiagnostics);
	ReturnValue_t generateSetStructurePacket(sid_t sid, bool isDiagnostics);
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
