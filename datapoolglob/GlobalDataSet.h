#ifndef FRAMEWORK_DATAPOOLGLOB_DATASET_H_
#define FRAMEWORK_DATAPOOLGLOB_DATASET_H_

#include <framework/datapool/PoolDataSetBase.h>

/**
 * @brief	The DataSet class manages a set of locally checked out variables
 * 			for the global data pool.
 * @details
 * This class uses the read-commit() semantic provided by the DataSetBase class.
 * It extends the base class by using the global data pool,
 * having a valid state and implementing lock und unlock calls for the global
 * datapool.
 *
 * For more information on how this class works, see the DataSetBase
 * documentation.
 * @author	Bastian Baetz
 * @ingroup data_pool
 */
class GlobDataSet: public PoolDataSetBase {
public:

	/**
	 * @brief	Creates an empty GlobDataSet. Use registerVariable or
	 * 			supply a pointer to this dataset to PoolVariable
	 * 			initializations to register pool variables.
	 */
	GlobDataSet();

	/**
	 * @brief	The destructor automatically manages writing the valid
	 * 			information of variables.
	 * @details
	 * In case the data set was read out, but not committed(indicated by state),
	 * the destructor parses all variables that are still registered to the set.
	 * For each, the valid flag in the data pool is set to "invalid".
	 */
	~GlobDataSet();

	/**
	 * Variant of method above which sets validity of all elements of the set.
	 * @param valid Validity information from PoolVariableIF.
	 * @return	- @c RETURN_OK if all variables were read successfully.
	 * 			- @c COMMITING_WITHOUT_READING if set was not read yet and
	 * 			     contains non write-only variables
	 */
	ReturnValue_t commit(bool valid, uint32_t lockTimeout = MutexIF::BLOCKING);
	ReturnValue_t commit(uint32_t lockTimeout = MutexIF::BLOCKING) override;

	/**
	 * Set all entries
	 * @param valid
	 */
	void setSetValid(bool valid);

	bool isValid() const override;

	/**
	 * Set the valid information of all variables contained in the set which
	 * are not read-only
	 *
	 * @param valid Validity information from PoolVariableIF.
	 */
	void setEntriesValid(bool valid);

	//!< This definition sets the maximum number of variables to
	//! register in one DataSet.
	static const uint8_t DATA_SET_MAX_SIZE = 63;

private:
	/**
	 * If the valid state of a dataset is always relevant to the whole
	 * data set we can use this flag.
	 */
	bool valid = false;

	/**
	 * @brief	This is a small helper function to facilitate locking
	 * 			the global data pool.
	 * @details
	 * It makes use of the lockDataPool method offered by the DataPool class.
	 */
	ReturnValue_t lockDataPool(uint32_t timeoutMs) override;
	/**
	 * @brief	This is a small helper function to facilitate
	 * 			unlocking the global data pool
	 * @details
	 * It makes use of the freeDataPoolLock method offered by the DataPool class.
	 */
	ReturnValue_t unlockDataPool() override;

	void handleAlreadyReadDatasetCommit();
	ReturnValue_t handleUnreadDatasetCommit();

	PoolVariableIF* registeredVariables[DATA_SET_MAX_SIZE];
};

#endif /* FRAMEWORK_DATAPOOLGLOB_DATASET_H_ */
