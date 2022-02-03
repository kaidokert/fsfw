#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETBASE_H_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETBASE_H_

#include <vector>

#include "MarkChangedIF.h"
#include "fsfw/datapool/DataSetIF.h"
#include "fsfw/datapool/PoolDataSetBase.h"
#include "localPoolDefinitions.h"

class LocalDataPoolManager;
class HasLocalDataPoolIF;
class PeriodicHousekeepingHelper;

/**
 * @brief	The LocalDataSet class manages a set of locally checked out
 *          variables for local data pools
 * @details
 * Extends the PoolDataSetBase class for local data pools by introducing
 * a validity state, a flag to mark the set as changed, and various other
 * functions to make it usable by the LocalDataPoolManager class.
 *
 * This class manages a list, where a set of local variables (or pool variables)
 * are registered. They are checked-out (i.e. their values are looked
 * up and copied) with the read call. After the user finishes working with the
 * pool variables, he can write back all variable values to the pool with
 * the commit call. The data set manages locking and freeing the local data
 * pools, to ensure thread-safety.
 *
 * Pool variables can be added to the dataset by using the constructor
 * argument of the pool variable or using the #registerVariable member function.
 *
 * An internal state manages usage of this class. Variables may only be
 * registered before any read call is made, and the commit call can only happen
 * after the read call.
 *
 * If pool variables are writable and not committed until destruction
 * of the set, the DataSet class automatically sets the valid flag in the
 * data pool to invalid (without) changing the variable's value.
 *
 * @ingroup data_pool
 */
class LocalPoolDataSetBase : public PoolDataSetBase, public MarkChangedIF {
  friend class LocalPoolDataSetAttorney;
  friend class PeriodicHousekeepingHelper;

 public:
  /**
   * @brief	Constructor for the creator of local pool data.
   * @details
   * This constructor also initializes the components required for
   * periodic handling.
   */
  LocalPoolDataSetBase(HasLocalDataPoolIF* hkOwner, uint32_t setId,
                       PoolVariableIF** registeredVariablesArray, const size_t maxNumberOfVariables,
                       bool periodicHandling = true);

  /**
   * @brief	Constructor for users of the local pool data, which need
   *          to access data created by one HK manager.
   * @details
   * Unlike the first constructor, no component for periodic handling
   * will be initiated.
   * @param sid Unique identifier of dataset consisting of object ID and
   * set ID.
   * @param registeredVariablesArray
   * @param maxNumberOfVariables
   */
  LocalPoolDataSetBase(sid_t sid, PoolVariableIF** registeredVariablesArray,
                       const size_t maxNumberOfVariables);

  /**
   * @brief	Simple constructor, if the dataset is not the owner by
   * 			a class with a HK manager.
   * @details
   * This constructor won't create components required for periodic handling
   * and it also won't try to deduce the HK manager because no SID is
   * supplied. This function should therefore be called by classes which need
   * to access pool variables from different creators.
   *
   * If the class is intended to access pool variables from different
   * creators, the third argument should be set to true. The mutex
   * properties can be set with #setReadCommitProtectionBehaviour .
   * @param registeredVariablesArray
   * @param maxNumberOfVariables
   * @param protectEveryReadCommitCall If the pool variables are created by
   * multiple creators, this flag can be set to protect all read and
   * commit calls separately.
   */
  LocalPoolDataSetBase(PoolVariableIF** registeredVariablesArray, const size_t maxNumberOfVariables,
                       bool protectEveryReadCommitCall = true);

  /**
   * @brief	The destructor automatically manages writing the valid
   * 			information of variables.
   * @details
   * In case the data set was read out, but not committed (indicated by state),
   * the destructor parses all variables that are still registered to the set.
   * For each, the valid flag in the data pool is set to "invalid".
   */
  ~LocalPoolDataSetBase();

  /* The copy constructor and assingment constructor are forbidden for now.
  The use-cases are limited and the first step would be to implement them properly for the
  base class */
  LocalPoolDataSetBase(const LocalPoolDataSetBase& otherSet) = delete;
  const LocalPoolDataSetBase& operator=(const LocalPoolDataSetBase& otherSet) = delete;

  /**
   * Helper functions used to set all currently contained variables to read-only.
   * It is recommended to call this in set constructors intended to be used
   * by data consumers to prevent accidentally changing pool data.
   */
  void setAllVariablesReadOnly();
  void setValidityBufferGeneration(bool withValidityBuffer);

  sid_t getSid() const;

  /** SerializeIF overrides */
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          SerializeIF::Endianness streamEndianness) const override;
  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                            SerializeIF::Endianness streamEndianness) override;
  size_t getSerializedSize() const override;

  /**
   * Special version of the serilization function which appends a
   * validity buffer at the end. Each bit of this validity buffer
   * denotes whether the container data set entries are valid from left
   * to right, MSB first. (length = ceil(N/8), N = number of pool variables)
   * @param buffer
   * @param size
   * @param maxSize
   * @param bigEndian
   * @param withValidityBuffer
   * @return
   */
  ReturnValue_t serializeWithValidityBuffer(uint8_t** buffer, size_t* size, size_t maxSize,
                                            SerializeIF::Endianness streamEndianness) const;
  ReturnValue_t deSerializeWithValidityBuffer(const uint8_t** buffer, size_t* size,
                                              SerializeIF::Endianness streamEndianness);
  ReturnValue_t serializeLocalPoolIds(uint8_t** buffer, size_t* size, size_t maxSize,
                                      SerializeIF::Endianness streamEndianness,
                                      bool serializeFillCount = true) const;
  uint8_t getLocalPoolIdsSerializedSize(bool serializeFillCount = true) const;

  /**
   * Set the dataset valid or invalid. These calls are mutex protected.
   * @param setEntriesRecursively
   * If this is true, all contained datasets will also be set recursively.
   */
  void setValidity(bool valid, bool setEntriesRecursively);
  bool isValid() const override;

  /**
   * These calls are mutex protected.
   * @param changed
   */
  void setChanged(bool changed) override;
  bool hasChanged() const override;

  object_id_t getCreatorObjectId();

  bool getReportingEnabled() const;

  /**
   * Returns the current periodic HK generation interval this set
   * belongs to a HK manager and the interval is not 0. Otherwise,
   * returns 0.0
   * @return
   */
  float getCollectionInterval() const;

  /**
   * @brief   Can be overwritten by a specific implementation of a dataset to print the set.
   */
  virtual void printSet();

 protected:
  sid_t sid;
  //! This mutex is used if the data is created by one object only.
  MutexIF* mutexIfSingleDataCreator = nullptr;

  bool diagnostic = false;
  void setDiagnostic(bool diagnostics);
  bool isDiagnostics() const;

  /**
   * Used for periodic generation.
   */
  bool reportingEnabled = false;
  void setReportingEnabled(bool enabled);

  void initializePeriodicHelper(float collectionInterval, dur_millis_t minimumPeriodicInterval,
                                uint8_t nonDiagIntervalFactor = 5);

  /**
   * If the valid state of a dataset is always relevant to the whole
   * data set we can use this flag.
   */
  bool valid = false;

  /**
   * Can be used to mark the dataset as changed, which is used
   * by the LocalDataPoolManager to send out update messages.
   */
  bool changed = false;

  /**
   * Specify whether the validity buffer is serialized too when serializing
   * or deserializing the packet. Each bit of the validity buffer will
   * contain the validity state of the pool variables from left to right.
   * The size of validity buffer thus will be ceil(N / 8) with N = number of
   * pool variables.
   */
  bool withValidityBuffer = true;

  /**
   * @brief	This is a small helper function to facilitate locking
   * 			the global data pool.
   * @details
   * It makes use of the lockDataPool method offered by the DataPool class.
   */
  ReturnValue_t lockDataPool(MutexIF::TimeoutType timeoutType, uint32_t timeoutMs) override;

  /**
   * @brief	This is a small helper function to facilitate
   * 			unlocking the global data pool
   * @details
   * It makes use of the freeDataPoolLock method offered by the DataPool class.
   */
  ReturnValue_t unlockDataPool() override;

  PeriodicHousekeepingHelper* periodicHelper = nullptr;
  LocalDataPoolManager* poolManager = nullptr;
};

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLDATASETBASE_H_ */
