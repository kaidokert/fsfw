#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_
#include <framework/datapool/DataSetIF.h>
#include <framework/serialize/SerializeIF.h>

/**
 * @brief	The LocalDataSet class manages a set of locally checked out variables
 * 			for local data pools
 * @details
 * This class manages a list, where a set of local variables (or pool variables)
 * are registered. They are checked-out (i.e. their values are looked
 * up and copied) with the read call. After the user finishes working with the
 * pool variables, he can write back all variable values to the pool with
 * the commit call. The data set manages locking and freeing the local data pools,
 * to ensure thread-safety.
 *
 * An internal state manages usage of this class. Variables may only be
 * registered before the read call is made, and the commit call only
 * after the read call.
 *
 * If pool variables are writable and not committed until destruction
 * of the set, the DataSet class automatically sets the valid flag in the
 * data pool to invalid (without) changing the variable's value.
 *
 * @ingroup data_pool
 */
class LocalDataSet:
		public DataSetIF,
		public HasReturnvaluesIF,
		public SerializeIF {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::DATA_SET_CLASS;
	static constexpr ReturnValue_t INVALID_PARAMETER_DEFINITION =
			MAKE_RETURN_CODE( 0x01 );
	static constexpr ReturnValue_t SET_WAS_ALREADY_READ = MAKE_RETURN_CODE( 0x02 );
	static constexpr ReturnValue_t COMMITING_WITHOUT_READING =
			MAKE_RETURN_CODE(0x03);

	static constexpr ReturnValue_t DATA_SET_UNINITIALIZED = MAKE_RETURN_CODE( 0x04 );
	static constexpr ReturnValue_t DATA_SET_FULL = MAKE_RETURN_CODE( 0x05 );
	static constexpr ReturnValue_t POOL_VAR_NULL = MAKE_RETURN_CODE( 0x06 );
	/**
	 * @brief	The constructor simply sets the fill_count to zero and sets
	 *  		the state to "uninitialized".
	 */
	LocalDataSet();

	/**
	 * @brief	This operation is used to register the local variables in the set.
	 * @details	It stores the pool variable pointer in a variable list.
	 */
	ReturnValue_t registerVariable(PoolVariableIF* variable) override;

	/**
	 * @brief	The destructor automatically manages writing the valid
	 * 			information of variables.
	 * @details
	 * In case the data set was read out, but not committed(indicated by state),
	 * the destructor parses all variables that are still registered to the set.
	 * For each, the valid flag in the data pool is set to "invalid".
	 */
	~LocalDataSet();

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
	 * @return	- @c RETURN_OK if all variables were read successfully.
	 * 			- @c INVALID_PARAMETER_DEFINITION if PID, size or type of the
	 * 					requested variable is invalid.
	 * 			- @c SET_WAS_ALREADY_READ if read() is called twice without
	 * 				 	calling commit() in between
	 */
	ReturnValue_t read();

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
	 * If the set does contain at least one variable which is not write-only commit()
	 * can only be called after read(). If the set only contains variables which are
	 * write only, commit() can be called without a preceding read() call.
	 * @return	- @c RETURN_OK if all variables were read successfully.
	 * 			- @c COMMITING_WITHOUT_READING if set was not read yet and
	 * 			  contains non write-only variables
	 */
	ReturnValue_t commit(void);

	/**
	 * Variant of method above which sets validity of all elements of the set.
	 * @param valid Validity information from PoolVariableIF.
	 * @return	- @c RETURN_OK if all variables were read successfully.
	 * 			- @c COMMITING_WITHOUT_READING if set was not read yet and
	 * 			     contains non write-only variables
	 */
	ReturnValue_t commit(bool valid);

	/**
	 * Set all entries
	 * @param valid
	 */
	void setSetValid(bool valid);

	/**
	 * Set the valid information of all variables contained in the set which
	 * are not read-only
	 *
	 * @param valid Validity information from PoolVariableIF.
	 */
	void setEntriesValid(bool valid);

	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size, bool bigEndian) const override;

	size_t getSerializedSize() const override;

	ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			bool bigEndian) override;
private:
	// SHOULDDO we could use a linked list of datapool variables
	//! This definition sets the maximum number of variables
	//! to register in one DataSet.
	static const uint8_t DATA_SET_MAX_SIZE = 63;
	/**
	 * @brief	This array represents all pool variables registered in this set.
	 */
	PoolVariableIF* registeredVariables[DATA_SET_MAX_SIZE];
	/**
	 * @brief	The fill_count attribute ensures that the variables register in
	 * 			the correct array position and that the maximum number of
	 * 			variables is not exceeded.
	 */
	uint16_t fill_count;

	/**
	 * States of the seet.
	 */
	enum States {
		DATA_SET_UNINITIALISED, //!< DATA_SET_UNINITIALISED
		DATA_SET_WAS_READ     //!< DATA_SET_WAS_READ
	};

	/**
	 * @brief	state manages the internal state of the data set,
	 *          which is important e.g. for the behavior on destruction.
	 */
	States state;

	/**
	 * If the valid state of a dataset is always relevant to the whole
	 * data set we can use this flag.
	 */
	bool valid = false;
	/**
	 * @brief	This is a small helper function to facilitate locking
	 * 			the underlying data data pool structure
	 * @details
	 * It makes use of the lockDataPool method offered by the DataPool class.
	 */
	ReturnValue_t lockDataPool() override;

	/**
	 * @brief	This is a small helper function to facilitate
	 * 			unlocking the underlying data data pool structure
	 * @details
	 * It makes use of the freeDataPoolLock method offered by the DataPool class.
	 */
	ReturnValue_t unlockDataPool() override;

	void handleAlreadyReadDatasetCommit();
	ReturnValue_t handleUnreadDatasetCommit();
};

#endif /* FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_ */
