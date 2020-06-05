#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_
#include <framework/datapool/DataSetBase.h>
#include <framework/datapool/DataSetIF.h>
#include <framework/housekeeping/HasHkPoolParametersIF.h>
#include <framework/serialize/SerializeIF.h>

class HousekeepingManager;

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
class LocalDataSet: public DataSetBase {
public:
	/**
	 * @brief	Constructor for the creator of local pool data.
	 * 			The constructor simply sets the fill_count to zero and sets
	 *  		the state to "uninitialized".
	 */
	LocalDataSet(HasHkPoolParametersIF* hkOwner);

	/**
	 * @brief	Constructor for users of local pool data. The passed pool
	 * 			owner should implement the HasHkPoolParametersIF.
	 * 			The constructor simply sets the fill_count to zero and sets
	 *  		the state to "uninitialized".
	 */
	LocalDataSet(object_id_t ownerId);

	/**
	 * @brief	The destructor automatically manages writing the valid
	 * 			information of variables.
	 * @details
	 * In case the data set was read out, but not committed(indicated by state),
	 * the destructor parses all variables that are still registered to the set.
	 * For each, the valid flag in the data pool is set to "invalid".
	 */
	~LocalDataSet();

protected:
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

	HousekeepingManager* hkManager;
};

#endif /* FRAMEWORK_DATAPOOLLOCAL_LOCALDATASET_H_ */
