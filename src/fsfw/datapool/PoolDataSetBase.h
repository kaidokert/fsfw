#ifndef FSFW_DATAPOOL_POOLDATASETBASE_H_
#define FSFW_DATAPOOL_POOLDATASETBASE_H_

#include "PoolDataSetIF.h"
#include "PoolVariableIF.h"
#include "fsfw/ipc/MutexIF.h"
#include "fsfw/serialize/SerializeIF.h"

/**
 * @brief	The DataSetBase class manages a set of locally checked out variables.
 * @details
 * This class manages a list, where a set of local variables (or pool variables)
 * are registered. They are checked-out (i.e. their values are looked
 * up and copied) with the read call. After the user finishes working with the
 * pool variables, he can write back all variable values to the pool with
 * the commit call. The data set manages locking and freeing the data pool,
 * to ensure that all values are read and written back at once.
 *
 * An internal state manages usage of this class. Variables may only be
 * registered before the read call is made, and the commit call only
 * after the read call.
 *
 * If pool variables are writable and not committed until destruction
 * of the set, the DataSet class automatically sets the valid flag in the
 * data pool to invalid (without) changing the variable's value.
 *
 * The base class lockDataPool und unlockDataPool implementation are empty
 * and should be implemented to protect the underlying pool type.
 * @author	Bastian Baetz
 * @ingroup data_pool
 */
class PoolDataSetBase : public PoolDataSetIF, public SerializeIF {
 public:
  /**
   * @brief	Creates an empty dataset. Use registerVariable or
   * 			supply a pointer to this dataset to PoolVariable
   * 			initializations to register pool variables.
   */
  PoolDataSetBase(PoolVariableIF** registeredVariablesArray, size_t maxFillCount);

  /* Forbidden for now */
  PoolDataSetBase(const PoolDataSetBase& otherSet) = delete;
  const PoolDataSetBase& operator=(const PoolDataSetBase& otherSet) = delete;

  ~PoolDataSetBase() override;

  /**
   * @brief	The read call initializes reading out all registered variables.
   * 			It is mandatory to call commit after every read call!
   * @details
   * It iterates through the list of registered variables and calls all read()
   * functions of the registered pool variables (which read out their values
   * from the data pool) which are not write-only.
   * In case of an error (e.g. a wrong data type, or an invalid data pool id),
   * the operation is aborted and @c INVALID_PARAMETER_DEFINITION returned.
   *
   * The data pool is locked during the whole read operation and
   * freed afterwards. It is mandatory to call commit after a read call,
   * even if the read operation is not successful!
   * @return
   * - @c returnvalue::OK if all variables were read successfully.
   * - @c INVALID_PARAMETER_DEFINITION if a pool entry does not exist or there
   *      is a type conflict.
   * - @c SET_WAS_ALREADY_READ if read() is called twice without calling
   *      commit() in between
   */
  virtual ReturnValue_t read(MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
                             uint32_t lockTimeout = 20) override;
  /**
   * @brief	The commit call initializes writing back the registered variables.
   * @details
   * It iterates through the list of registered variables and calls the
   * commit() method of the remaining registered variables (which write back
   * their values to the pool).
   *
   * The data pool is locked during the whole commit operation and
   * freed afterwards. The state changes to "was committed" after this operation.
   *
   * If the set does contain at least one variable which is not write-only
   * commit() can only be called after read(). If the set only contains
   * variables which are write only, commit() can be called without a
   * preceding read() call. Every read call must be followed by a commit call!
   * @return	- @c returnvalue::OK if all variables were read successfully.
   * 			- @c COMMITING_WITHOUT_READING if set was not read yet and
   * 			  contains non write-only variables
   */
  virtual ReturnValue_t commit(MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
                               uint32_t lockTimeout = 20) override;

  /**
   * Register the passed pool variable instance into the data set.
   * @param variable
   * @return
   */
  virtual ReturnValue_t registerVariable(PoolVariableIF* variable) override;

  /**
   * Provides the means to lock the underlying data structure to ensure
   * thread-safety. Default implementation is empty
   * @return Always returns -@c returnvalue::OK
   */
  virtual ReturnValue_t lockDataPool(
      MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
      uint32_t timeoutMs = 20) override;
  /**
   * Provides the means to unlock the underlying data structure to ensure
   * thread-safety. Default implementation is empty
   * @return Always returns -@c returnvalue::OK
   */
  virtual ReturnValue_t unlockDataPool() override;

  virtual uint16_t getFillCount() const override;

  /* SerializeIF implementations */
  virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size, const size_t maxSize,
                                  SerializeIF::Endianness streamEndianness) const override;
  virtual size_t getSerializedSize() const override;
  virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
                                    SerializeIF::Endianness streamEndianness) override;

  /**
   * Can be used to individually protect every read and commit call.
   * @param protectEveryReadCommit
   * @param mutexTimeout
   */
  void setReadCommitProtectionBehaviour(
      bool protectEveryReadCommit, MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING,
      uint32_t mutexTimeout = 20);

 protected:
  /**
   * @brief	The fill_count attribute ensures that the variables
   * 			register in the correct array position and that the maximum
   * 			number of variables is not exceeded.
   */
  uint16_t fillCount = 0;
  /**
   * States of the seet.
   */
  enum class States {
    STATE_SET_UNINITIALISED,  //!< DATA_SET_UNINITIALISED
    STATE_SET_WAS_READ        //!< DATA_SET_WAS_READ
  };
  /**
   * @brief	state manages the internal state of the data set,
   *          which is important e.g. for the behavior on destruction.
   */
  States state = States::STATE_SET_UNINITIALISED;

  /**
   * @brief	This array represents all pool variables registered in this set.
   * Child classes can use a static or dynamic container to create
   * an array of registered variables and assign the first entry here.
   */
  PoolVariableIF** registeredVariables = nullptr;
  const size_t maxFillCount = 0;

  void setContainer(PoolVariableIF** variablesContainer);
  PoolVariableIF** getContainer() const;

 private:
  bool protectEveryReadCommitCall = false;
  MutexIF::TimeoutType timeoutTypeForSingleVars = MutexIF::TimeoutType::WAITING;
  uint32_t mutexTimeoutForSingleVars = 20;

  ReturnValue_t readVariable(uint16_t count);
  void handleAlreadyReadDatasetCommit(
      MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING, uint32_t timeoutMs = 20);
  ReturnValue_t handleUnreadDatasetCommit(
      MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::WAITING, uint32_t timeoutMs = 20);
};

#endif /* FSFW_DATAPOOL_POOLDATASETBASE_H_ */
