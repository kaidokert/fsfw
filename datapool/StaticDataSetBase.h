#ifndef FRAMEWORK_DATAPOOL_STATICDATASETBASE_H_
#define FRAMEWORK_DATAPOOL_STATICDATASETBASE_H_
#include <framework/datapool/DataSetIF.h>
#include <framework/datapool/PoolVariableIF.h>
#include <framework/ipc/MutexIF.h>

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
class DataSetBase: public DataSetIF,
		public SerializeIF,
		public HasReturnvaluesIF {
public:

	/**
	 * @brief	Creates an empty dataset. Use registerVariable or
	 * 			supply a pointer to this dataset to PoolVariable
	 * 			initializations to register pool variables.
	 */
	DataSetBase();
	virtual~ DataSetBase();

	/**
	 * @brief	The read call initializes reading out all registered variables.
	 * @details
	 * It iterates through the list of registered variables and calls all read()
	 * functions of the registered pool variables (which read out their values
	 * from the data pool) which are not write-only.
	 * In case of an error (e.g. a wrong data type, or an invalid data pool id),
	 * the operation is aborted and @c INVALID_PARAMETER_DEFINITION returned.
	 *
	 * The data pool is locked during the whole read operation and
	 * freed afterwards.The state changes to "was written" after this operation.
	 * @return
	 * - @c RETURN_OK if all variables were read successfully.
	 * - @c INVALID_PARAMETER_DEFINITION if PID, size or type of the
	 * requested variable is invalid.
	 * - @c SET_WAS_ALREADY_READ if read() is called twice without calling
	 * commit() in between
	 */
	virtual ReturnValue_t read(uint32_t lockTimeout =
			MutexIF::BLOCKING) override;
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
	 * preceding read() call.
	 * @return	- @c RETURN_OK if all variables were read successfully.
	 * 			- @c COMMITING_WITHOUT_READING if set was not read yet and
	 * 			  contains non write-only variables
	 */
	virtual ReturnValue_t commit(uint32_t lockTimeout =
			MutexIF::BLOCKING) override;

	/**
	 * Register the passed pool variable instance into the data set.
	 * @param variable
	 * @return
	 */
	virtual ReturnValue_t registerVariable( PoolVariableIF* variable) override;
	/**
	 * Provides the means to lock the underlying data structure to ensure
	 * thread-safety. Default implementation is empty
	 * @return Always returns -@c RETURN_OK
	 */
	virtual ReturnValue_t lockDataPool(uint32_t timeoutMs =
			MutexIF::BLOCKING) override;
	/**
	 * Provides the means to unlock the underlying data structure to ensure
	 * thread-safety. Default implementation is empty
	 * @return Always returns -@c RETURN_OK
	 */
	virtual ReturnValue_t unlockDataPool() override;

	virtual uint16_t getFillCount() const;

	/* SerializeIF implementations */
	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
				const size_t maxSize, bool bigEndian) const override;
	virtual size_t getSerializedSize() const override;
	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			bool bigEndian) override;

	// SHOULDDO we could use a linked list of datapool variables
	//!< This definition sets the maximum number of variables to
	//! register in one DataSet.
	static const uint8_t DATA_SET_MAX_SIZE = 63;

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
		DATA_SET_UNINITIALISED, //!< DATA_SET_UNINITIALISED
		DATA_SET_WAS_READ     //!< DATA_SET_WAS_READ
	};
	/**
	 * @brief	state manages the internal state of the data set,
	 *          which is important e.g. for the behavior on destruction.
	 */
	States state = States::DATA_SET_UNINITIALISED;

	/**
	 * @brief	This array represents all pool variables registered in this set.
	 */
	PoolVariableIF* registeredVariables[DATA_SET_MAX_SIZE] = { };

private:
	ReturnValue_t readVariable(uint16_t count);
	void handleAlreadyReadDatasetCommit(uint32_t lockTimeout);
	ReturnValue_t handleUnreadDatasetCommit(uint32_t lockTimeout);
};

#endif /* FRAMEWORK_DATAPOOL_STATICDATASETBASE_H_ */
