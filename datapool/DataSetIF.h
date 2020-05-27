#ifndef DATASETIF_H_
#define DATASETIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
class PoolVariableIF;

/**
 * @brief	This class defines a small interface to register on a DataSet.
 *
 * @details
 * Currently, the only purpose of this interface is to provide a
 * method for locally checked-out variables to register on a data set.
 * Still, it may become useful for other purposes as well.
 * @author 	Bastian Baetz
 * @ingroup data_pool
 */
class DataSetIF {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::DATA_SET_CLASS;
	static constexpr ReturnValue_t INVALID_PARAMETER_DEFINITION =
			MAKE_RETURN_CODE( 0x01 );
	static constexpr ReturnValue_t SET_WAS_ALREADY_READ = MAKE_RETURN_CODE( 0x02 );
	static constexpr ReturnValue_t COMMITING_WITHOUT_READING =
			MAKE_RETURN_CODE(0x03);

	static constexpr ReturnValue_t DATA_SET_UNINITIALISED = MAKE_RETURN_CODE( 0x04 );
	static constexpr ReturnValue_t DATA_SET_FULL = MAKE_RETURN_CODE( 0x05 );
	static constexpr ReturnValue_t POOL_VAR_NULL = MAKE_RETURN_CODE( 0x06 );

	/**
	 * @brief	This is an empty virtual destructor,
	 * 			as it is proposed for C++ interfaces.
	 */
	virtual ~DataSetIF() {}

	/**
	 * @brief	This operation provides a method to register local data pool
	 * 			variables to register in a data set by passing itself
	 * 			to this DataSet operation.
	 */
	virtual ReturnValue_t registerVariable( PoolVariableIF* variable ) = 0;

	/**
	 * @brief 	Most underlying data structures will have a pool like structure
	 * 			and will require a lock and unlock mechanism to ensure
	 * 			thread-safety
	 * @return Lock operation result
	 */
	virtual ReturnValue_t lockDataPool() = 0;
	/**
	 * @brief   Unlock call corresponding to the lock call.
	 * @return Unlock operation result
	 */
	virtual ReturnValue_t unlockDataPool() = 0;
};

#endif /* DATASETIF_H_ */
