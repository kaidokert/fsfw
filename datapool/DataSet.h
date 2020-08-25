/*
 * \file	DataSet.h
 *
 * \brief	This file contains the DataSet class and a small structure called DataSetContent.
 *
 * \date	10/17/2012
 *
 * \author	Bastian Baetz
 *
 */

#ifndef DATASET_H_
#define DATASET_H_

#include "DataPool.h"
#include "DataSetIF.h"
#include "PoolRawAccess.h"
#include "PoolVariable.h"
#include "PoolVarList.h"
#include "PoolVector.h"
#include "../serialize/SerializeAdapter.h"
/**
 * \brief	The DataSet class manages a set of locally checked out variables.
 *
 * \details	This class manages a list, where a set of local variables (or pool variables) are
 * 			registered. They are checked-out (i.e. their values are looked up and copied)
 * 			with the read call. After the user finishes working with the pool variables,
 * 			he can write back all variable values to the pool with the commit call.
 * 			The data set manages locking and freeing the data pool, to ensure that all values
 * 			are read and written back at once.
 * 			An internal state manages usage of this class. Variables may only be registered before
 * 			the read call is made, and the commit call only after the read call.
 * 			If pool variables are writable and not committed until destruction of the set, the
 * 			DataSet class automatically sets the valid flag in the data pool to invalid (without)
 * 			changing the variable's value.
 *
 *	\ingroup data_pool
 */
class DataSet: public DataSetIF, public HasReturnvaluesIF, public SerializeIF {
private:
	//SHOULDDO we could use a linked list of datapool variables
	static const uint8_t DATA_SET_MAX_SIZE = 63; //!< This definition sets the maximum number of variables to register in one DataSet.

	/**
	 * \brief	This array represents all pool variables registered in this set.
	 * \details	It has a maximum size of DATA_SET_MAX_SIZE.
	 */
	PoolVariableIF* registeredVariables[DATA_SET_MAX_SIZE];
	/**
	 * \brief	The fill_count attribute ensures that the variables register in the correct array
	 * 			position and that the maximum number of variables is not exceeded.
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
	 * \brief	state manages the internal state of the data set, which is important e.g. for the
	 * 			behavior on destruction.
	 */
	States state;
	/**
	 * \brief	This is a small helper function to facilitate locking the global data pool.
	 * \details	It makes use of the lockDataPool method offered by the DataPool class.
	 */
	uint8_t lockDataPool();
	/**
	 * \brief	This is a small helper function to facilitate unlocking the global data pool.
	 * \details	It makes use of the freeDataPoolLock method offered by the DataPool class.
	 */
	uint8_t freeDataPoolLock();

public:
	static const uint8_t INTERFACE_ID = CLASS_ID::DATA_SET_CLASS;
	static const ReturnValue_t INVALID_PARAMETER_DEFINITION =
			MAKE_RETURN_CODE( 0x01 );
	static const ReturnValue_t SET_WAS_ALREADY_READ = MAKE_RETURN_CODE( 0x02 );
	static const ReturnValue_t COMMITING_WITHOUT_READING =
			MAKE_RETURN_CODE(0x03);

	/**
	 * \brief	The constructor simply sets the fill_count to zero and sets the state to "uninitialized".
	 */
	DataSet();
	/**
	 * \brief	The destructor automatically manages writing the valid information of variables.
	 * \details	In case the data set was read out, but not committed (indicated by state),
	 * 			the destructor parses all variables that are still registered to the set.
	 * 			For each, the valid flag in the data pool is set to "invalid".
	 */
	~DataSet();
	/**
	 * \brief	The read call initializes reading out all registered variables.
	 * \details	It iterates through the list of registered variables and calls all read()
	 * 			functions of the registered pool variables (which read out their values from the
	 * 			data pool) which are not write-only. In case of an error (e.g. a wrong data type,
	 * 			or an invalid data pool id), the operation is aborted and
	 * 			\c INVALID_PARAMETER_DEFINITION returned.
	 * 			The data pool is locked during the whole read operation and freed afterwards.
	 * 			The state changes to "was written" after this operation.
	 * \return	- \c RETURN_OK if all variables were read successfully.
	 * 			- \c INVALID_PARAMETER_DEFINITION if PID, size or type of the
	 * 					requested variable is invalid.
	 * 			- \c SET_WAS_ALREADY_READ if read() is called twice without calling
	 * 					commit() in between
	 */
	ReturnValue_t read();
	/**
	 * \brief	The commit call initializes writing back the registered variables.
	 * \details	It iterates through the list of registered variables and calls
	 * 			the commit() method of the remaining registered variables (which write back
	 * 			their values to the pool).
	 * 			The data pool is locked during the whole commit operation and freed afterwards.
	 * 			The state changes to "was committed" after this operation.
	 * 			If the set does contain at least one variable which is not write-only commit()
	 * 			can only be called after read(). If the set only contains variables which are
	 * 			write only, commit() can be called without a preceding read() call.
	 * \return	- \c RETURN_OK if all variables were read successfully.
	 * 			- \c COMMITING_WITHOUT_READING if set was not read yet and contains non write-only
	 * 				variables
	 */
	ReturnValue_t commit(void);
	/**
	 * Variant of method above which sets validity of all elements of the set.
	 * @param valid Validity information from PoolVariableIF.
	 * \return	- \c RETURN_OK if all variables were read successfully.
	 * 			- \c COMMITING_WITHOUT_READING if set was not read yet and contains non write-only
	 * 				variables
	 */
	ReturnValue_t commit(uint8_t valid);
	/**
	 * \brief	This operation is used to register the local variables in the set.
	 * \details	It copies all required information to the currently
	 * 			free space in the registeredVariables list.
	 */
	void registerVariable(PoolVariableIF* variable);

	/**
	 * Set the valid information of all variables contained in the set which are not readonly
	 *
	 * @param valid Validity information from PoolVariableIF.
	 */
	void setValid(uint8_t valid);

	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const override;

	size_t getSerializedSize() const override;

	ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) override;

};

#endif /* DATASET_H_ */
