#ifndef FSFW_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_
#define FSFW_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_

#include <map>
#include <vector>

#include "AccessLocalPoolF.h"
#include "ProvidesDataPoolSubscriptionIF.h"
#include "fsfw/datapool/DataSetIF.h"
#include "fsfw/datapool/PoolEntry.h"
#include "fsfw/housekeeping/AcceptsHkPacketsIF.h"
#include "fsfw/housekeeping/HousekeepingMessage.h"
#include "fsfw/housekeeping/HousekeepingPacketDownlink.h"
#include "fsfw/housekeeping/PeriodicHousekeepingHelper.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/ipc/MutexIF.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

namespace Factory {
void setStaticFrameworkObjectIds();
}

class LocalPoolDataSetBase;
class HousekeepingSnapshot;
class HasLocalDataPoolIF;
class LocalDataPool;

/**
 * @brief 		This class is the managing instance for the local data pool.
 * @details
 * The actual data pool structure is a member of this class. Any class which
 * has a local data pool shall have this manager class as a member and implement
 * the HasLocalDataPoolIF.
 *
 * The manager offers some adaption points and functions which can be used
 * by the owning class to simplify data handling significantly.
 *
 * Please ensure that both initialize and initializeAfterTaskCreation are
 * called at some point by the owning class in the respective functions of the
 * same name!
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
class LocalDataPoolManager : public ProvidesDataPoolSubscriptionIF, public AccessPoolManagerIF {
  friend void(Factory::setStaticFrameworkObjectIds)();
  //! Some classes using the pool manager directly need to access class internals of the
  //! manager. The attorney provides granular control of access to these internals.
  friend class LocalDpManagerAttorney;

 public:
  static constexpr uint8_t INTERFACE_ID = CLASS_ID::HOUSEKEEPING_MANAGER;

  static constexpr ReturnValue_t QUEUE_OR_DESTINATION_INVALID = MAKE_RETURN_CODE(0);
  static constexpr ReturnValue_t WRONG_HK_PACKET_TYPE = MAKE_RETURN_CODE(1);
  static constexpr ReturnValue_t REPORTING_STATUS_UNCHANGED = MAKE_RETURN_CODE(2);
  static constexpr ReturnValue_t PERIODIC_HELPER_INVALID = MAKE_RETURN_CODE(3);
  static constexpr ReturnValue_t POOLOBJECT_NOT_FOUND = MAKE_RETURN_CODE(4);
  static constexpr ReturnValue_t DATASET_NOT_FOUND = MAKE_RETURN_CODE(5);

  /**
   * This constructor is used by a class which wants to implement
   * a personal local data pool. The queueToUse can be supplied if it
   * is already known.
   *
   * initialize() has to be called in any case before using the object!
   * @param owner
   * @param queueToUse
   * @param appendValidityBuffer Specify whether a buffer containing the
   * validity state is generated  when serializing or deserializing packets.
   */
  LocalDataPoolManager(HasLocalDataPoolIF* owner, MessageQueueIF* queueToUse,
                       bool appendValidityBuffer = true);
  ~LocalDataPoolManager() override;

  void setHkDestinationId(MessageQueueId_t hkDestId);

  /**
   * Assigns the queue to use. Make sure to call this in the #initialize
   * function of the owner.
   * @param queueToUse
   * @param nonDiagInvlFactor See #setNonDiagnosticIntervalFactor doc
   * @return
   */
  ReturnValue_t initialize(MessageQueueIF* queueToUse);

  /**
   * Initializes the map by calling the map initialization function and
   * setting the periodic factor for non-diagnostic packets.
   * Don't forget to call this in the #initializeAfterTaskCreation call of
   * the owner, otherwise the map will be invalid!
   * @param nonDiagInvlFactor
   * @return
   */
  ReturnValue_t initializeAfterTaskCreation(uint8_t nonDiagInvlFactor = 5);

  /**
   * @brief   This should be called in the periodic handler of the owner.
   * @details
   * This in generally called in the #performOperation function of the owner.
   * It performs all the periodic functionalities of the data pool manager,
   * for example generating periodic HK packets.
   * Marked virtual as an adaption point for custom data pool managers.
   * @return
   */
  virtual ReturnValue_t performHkOperation();

  /**
   * @brief   Subscribe for a notification message which will be sent
   *          if a dataset has changed.
   * @details
   * This subscription mechanism will generally be used internally by
   * other software components.
   * @param setId     Set ID of the set to receive update messages from.
   * @param destinationObject
   * @param targetQueueId
   * @param generateSnapshot If this is set to true, a copy of the current
   * data with a timestamp will be generated and sent via message.
   * Otherwise, only an notification message is sent.
   * @return
   */
  ReturnValue_t subscribeForSetUpdateMessage(uint32_t setId, object_id_t destinationObject,
                                             MessageQueueId_t targetQueueId,
                                             bool generateSnapshot) override;

  /**
   * @brief   Subscribe for an notification message which will be sent if a
   *          pool variable has changed.
   * @details
   * This subscription mechanism will generally be used internally by
   * other software components.
   * @param localPoolId Pool ID of the pool variable
   * @param destinationObject
   * @param targetQueueId
   * @param generateSnapshot If this is set to true, a copy of the current
   * data with a timestamp will be generated and sent via message.
   * Otherwise, only an notification message is sent.
   * @return
   */
  ReturnValue_t subscribeForVariableUpdateMessage(lp_id_t localPoolId,
                                                  object_id_t destinationObject,
                                                  MessageQueueId_t targetQueueId,
                                                  bool generateSnapshot) override;

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
   * @brief   The manager is also able to handle housekeeping messages.
   * @details
   * This most commonly is used to handle messages for the housekeeping
   * interface, but the manager is also able to handle update notifications
   * and calls a special function which can be overriden by a child class
   * to handle data set or pool variable updates. This is relevant
   * for classes like controllers which have their own local datapool
   * but pull their data from other local datapools.
   * @param message
   * @return
   */
  virtual ReturnValue_t handleHousekeepingMessage(CommandMessage* message);

  /**
   * Generate a housekeeping packet with a given SID.
   * @param sid
   * @return
   */
  ReturnValue_t generateHousekeepingPacket(sid_t sid, LocalPoolDataSetBase* dataSet,
                                           bool forDownlink,
                                           MessageQueueId_t destination = MessageQueueIF::NO_QUEUE);

  HasLocalDataPoolIF* getOwner();

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
  enum class ReportingType : uint8_t {
    //! Periodic generation of HK packets.
    PERIODIC,
    //! Housekeeping packet will be generated if values have changed.
    UPDATE_HK,
    //! Update notification will be sent out as message.
    UPDATE_NOTIFICATION,
    //! Notification will be sent out as message and a snapshot of the
    //! current data will be generated.
    UPDATE_SNAPSHOT,
  };

  /** Different data types are possible in the HK receiver map. For example, updates can be
  requested for full datasets or for single pool variables. Periodic reporting is only possible
  for data sets. */
  enum class DataType : uint8_t { LOCAL_POOL_VARIABLE, DATA_SET };

  /* Copying forbidden */
  LocalDataPoolManager(const LocalDataPoolManager&) = delete;
  LocalDataPoolManager operator=(const LocalDataPoolManager&) = delete;

  /**
   * This function can be used to clear the receivers list. This is
   * intended for test functions and not for regular operations, because
   * the insertion operations allocate dynamically.
   */
  void clearReceiversList();

  [[nodiscard]] object_id_t getCreatorObjectId() const;

  /**
   * Get the pointer to the mutex. Can be used to lock the data pool
   * externally. Use with care and don't forget to unlock locked mutexes!
   * For now, only friend classes can accss this function.
   * @return
   */
  MutexIF* getMutexHandle();

  LocalDataPoolManager* getPoolManagerHandle() override;
  ReturnValue_t subscribeForRegularPeriodicPacket(subdp::RegularHkPeriodicParams params) override;
  ReturnValue_t subscribeForDiagPeriodicPacket(subdp::DiagnosticsHkPeriodicParams params) override;

  ReturnValue_t subscribeForRegularUpdatePacket(subdp::RegularHkUpdateParams params) override;
  ReturnValue_t subscribeForDiagUpdatePacket(subdp::DiagnosticsHkUpdateParams params) override;

 protected:
  ReturnValue_t subscribeForPeriodicPacket(subdp::ParamsBase& params);
  ReturnValue_t subscribeForUpdatePacket(subdp::ParamsBase& params);

  /** Core data structure for the actual pool data */
  localpool::DataPool localPoolMap;
  /** Every housekeeping data manager has a mutex to protect access
  to it's data pool. */
  MutexIF* mutex = nullptr;

  /** The class which actually owns the manager (and its datapool). */
  HasLocalDataPoolIF* owner = nullptr;

  uint8_t nonDiagnosticIntervalFactor = 0;

  /** Default receiver for periodic HK packets */
  static object_id_t defaultHkDestination;
  MessageQueueId_t hkDestinationId = MessageQueueIF::NO_QUEUE;

  union DataId {
    DataId() : sid(){};
    sid_t sid;
    lp_id_t localPoolId;
  };

  /** The data pool manager will keep an internal map of HK receivers. */
  struct HkReceiver {
    /** Object ID of receiver */
    object_id_t objectId = objects::NO_OBJECT;

    DataType dataType = DataType::DATA_SET;
    DataId dataId;

    ReportingType reportingType = ReportingType::PERIODIC;
    MessageQueueId_t destinationQueue = MessageQueueIF::NO_QUEUE;
  };

  /** This vector will contain the list of HK receivers. */
  using HkReceivers = std::vector<struct HkReceiver>;

  HkReceivers hkReceivers;

  struct HkUpdateResetHelper {
    DataType dataType = DataType::DATA_SET;
    DataId dataId;
    uint8_t updateCounter;
    uint8_t currentUpdateCounter;
  };

  using HkUpdateResetList = std::vector<struct HkUpdateResetHelper>;
  /** This list is used to manage creating multiple update packets and only resetting
  the update flag if all of them were created. */
  HkUpdateResetList hkUpdateResetList = HkUpdateResetList();

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
   * Read a variable by supplying its local pool ID and assign the pool
   * entry to the supplied PoolEntry pointer. The type of the pool entry
   * is deduced automatically. This call is not thread-safe!
   * For now, only classes designated by the LocalDpManagerAttorney may use this function.
   * @tparam T Type of the pool entry
   * @param localPoolId Pool ID of the variable to read
   * @param poolVar [out] Corresponding pool entry will be assigned to the
   * 						supplied pointer.
   * @return
   */
  template <class T>
  ReturnValue_t fetchPoolEntry(lp_id_t localPoolId, PoolEntry<T>** poolEntry);

  /**
   * This function is used to fill the local data pool map with pool
   * entries. It should only be called once by the pool owner.
   * @param localDataPoolMap
   * @return
   */
  ReturnValue_t initializeHousekeepingPoolEntriesOnce();

  MutexIF* getLocalPoolMutex() override;

  ReturnValue_t serializeHkPacketIntoStore(HousekeepingPacketDownlink& hkPacket,
                                           store_address_t& storeId, bool forDownlink,
                                           size_t* serializedSize);

  void performPeriodicHkGeneration(HkReceiver& hkReceiver);
  ReturnValue_t togglePeriodicGeneration(sid_t sid, bool enable, bool isDiagnostics);
  ReturnValue_t changeCollectionInterval(sid_t sid, float newCollectionInterval,
                                         bool isDiagnostics);
  ReturnValue_t generateSetStructurePacket(sid_t sid, bool isDiagnostics);

  void handleHkUpdateResetListInsertion(DataType dataType, DataId dataId);
  void handleChangeResetLogic(DataType type, DataId dataId, MarkChangedIF* toReset);
  void resetHkUpdateResetHelper();

  ReturnValue_t handleHkUpdate(HkReceiver& hkReceiver, ReturnValue_t& status);
  ReturnValue_t handleNotificationUpdate(HkReceiver& hkReceiver, ReturnValue_t& status);
  ReturnValue_t handleNotificationSnapshot(HkReceiver& hkReceiver, ReturnValue_t& status);
  ReturnValue_t addUpdateToStore(HousekeepingSnapshot& updatePacket, store_address_t& storeId);

  void printWarningOrError(sif::OutputTypes outputType, const char* functionName,
                           ReturnValue_t errorCode = returnvalue::FAILED,
                           const char* errorPrint = nullptr);
};

template <class T>
inline ReturnValue_t LocalDataPoolManager::fetchPoolEntry(lp_id_t localPoolId,
                                                          PoolEntry<T>** poolEntry) {
  if (poolEntry == nullptr) {
    return returnvalue::FAILED;
  }

  auto poolIter = localPoolMap.find(localPoolId);
  if (poolIter == localPoolMap.end()) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "fetchPoolEntry",
                        localpool::POOL_ENTRY_NOT_FOUND);
    return localpool::POOL_ENTRY_NOT_FOUND;
  }

  *poolEntry = dynamic_cast<PoolEntry<T>*>(poolIter->second);
  if (*poolEntry == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "fetchPoolEntry",
                        localpool::POOL_ENTRY_TYPE_CONFLICT);
    return localpool::POOL_ENTRY_TYPE_CONFLICT;
  }
  return returnvalue::OK;
}

#endif /* FSFW_DATAPOOLLOCAL_LOCALDATAPOOLMANAGER_H_ */
