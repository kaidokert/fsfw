#ifndef FRAMEWORK_HK_HOUSEKEEPINGHELPER_H_
#define FRAMEWORK_HK_HOUSEKEEPINGHELPER_H_
#include <framework/datapool/DataSetIF.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/housekeeping/HasHkPoolParametersIF.h>
#include <framework/ipc/MutexIF.h>

#include <framework/datapool/PoolEntry.h>
#include <framework/ipc/CommandMessage.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/ipc/MutexHelper.h>

#include <map>


class HousekeepingManager {
public:
	static constexpr float MINIMAL_SAMPLING_FREQUENCY = 0.2;

	HousekeepingManager(HasHkPoolParametersIF* owner);
	virtual~ HousekeepingManager();

	MutexIF* getMutexHandle();

	// propably will just call respective local data set functions.
	void generateHousekeepingPacket(DataSetIF* dataSet);
	ReturnValue_t handleHousekeepingMessage(CommandMessage* message);

	/**
	 * Read a variable by supplying its local pool ID and assign the pool
	 * entry to the supplied PoolEntry pointer. The type of the pool entry
	 * is deduced automatically. This call is not thread-safe!
	 * @tparam T Type of the pool entry
	 * @param localPoolId Pool ID of the variable to read
	 * @param poolVar [out] Corresponding pool entry will be assigned to the
	 * 						supplied pointer.
	 * @return
	 */
	template <class T>
	ReturnValue_t fetchPoolEntry(lp_id_t localPoolId, PoolEntry<T> *poolEntry);
	void setMinimalSamplingFrequency(float frequencySeconds);

	/**
	 * This function is used to fill the local data pool map with pool
	 * entries. The default implementation is empty.
	 * @param localDataPoolMap
	 * @return
	 */
	ReturnValue_t initializeHousekeepingPoolEntriesOnce();

	void setHkPacketQueue(MessageQueueIF* msgQueue);
private:
	//! this depends on the PST frequency.. maybe it would be better to just
	//! set this manually with a global configuration value which is also
	//! passed to the PST. Or force setting this in device handler.
	float samplingFrequency = MINIMAL_SAMPLING_FREQUENCY;

	//! This is the map holding the actual data. Should only be initialized
	//! once !
	bool mapInitialized = false;
	LocalDataPoolMap localDpMap;

	//! Every housekeeping data manager has a mutex to protect access
	//! to it's data pool.
	MutexIF * mutex = nullptr;

	//! The class which actually owns the manager (and its datapool).
	HasHkPoolParametersIF* owner = nullptr;

	//! Used for replies.
	//! (maybe we dont need this, the sender can be retrieved from command
	//! message..)
	MessageQueueIF* hkReplyQueue = nullptr;
	//! Used for HK packets, which are generated without requests.
	MessageQueueIF* hkPacketQueue = nullptr;
};

template<class T> inline
ReturnValue_t HousekeepingManager::fetchPoolEntry(lp_id_t localPoolId,
		PoolEntry<T> *poolEntry) {
	auto poolIter = localDpMap.find(localPoolId);
	if (poolIter == localDpMap.end()) {
		// todo: special returnvalue.
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	poolEntry = dynamic_cast< PoolEntry<T>* >(poolIter->second);
	if(poolEntry == nullptr) {
		// todo: special returnvalue.
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

#endif /* FRAMEWORK_HK_HOUSEKEEPINGHELPER_H_ */
