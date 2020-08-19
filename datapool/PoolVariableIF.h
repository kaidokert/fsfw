/*
 * \file	PoolVariableIF.h
 *
 * \brief	This file contains the interface definition for pool variables.
 *
 * \date	10/17/2012
 *
 * \author	Bastian Baetz
 */

#ifndef POOLVARIABLEIF_H_
#define POOLVARIABLEIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "../serialize/SerializeIF.h"

/**
 * \brief	This interface is used to control local data pool variable representations.
 *
 * \details	To securely handle data pool variables, all pool entries are locally managed by
 * 			data pool variable access classes, which are called pool variables. To ensure a
 * 			common state of a set of variables needed in a function, these local pool variables
 * 			again are managed by other classes, e.g. the DataSet. This interface provides unified
 * 			access to local pool variables for such manager classes.
 *	\ingroup data_pool
 */
class PoolVariableIF : public SerializeIF {
	friend class DataSet;
protected:
	/**
	 * \brief	The commit call shall write back a newly calculated local value to the data pool.
	 */
	virtual ReturnValue_t commit() = 0;
	/**
	 * \brief	The read call shall read the value of this parameter from the data pool and store
	 * 			the content locally.
	 */
	virtual ReturnValue_t read() = 0;
public:
	static const uint8_t VALID = 1;
	static const uint8_t INVALID = 0;
	static const uint32_t NO_PARAMETER = 0;
	enum ReadWriteMode_t {
		VAR_READ, VAR_WRITE, VAR_READ_WRITE
	};

	/**
	 * \brief	This is an empty virtual destructor, as it is proposed for C++ interfaces.
	 */
	virtual ~PoolVariableIF() {
	}
	/**
	 * \brief	This method returns if the variable is write-only, read-write or read-only.
	 */
	virtual ReadWriteMode_t getReadWriteMode() const = 0;
	/**
	 * \brief	This operation shall return the data pool id of the variable.
	 */
	virtual uint32_t getDataPoolId() const = 0;
	/**
	 * \brief	With this call, the valid information of the variable is returned.
	 */
	virtual bool isValid() const = 0;
	/**
	 * \brief	With this call, the valid information of the variable is set.
	 */
	virtual void setValid(uint8_t validity) = 0;

};

#endif /* POOLVARIABLEIF_H_ */
