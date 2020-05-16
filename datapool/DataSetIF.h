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

	virtual ReturnValue_t lockDataPool() = 0;
	virtual ReturnValue_t unlockDataPool() = 0;
};

#endif /* DATASETIF_H_ */
