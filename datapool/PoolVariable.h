/*
 * \file	PoolVariable.h
 *
 * \brief	This file contains the PoolVariable class, which locally represents a non-array data pool variable.
 *
 * \date	10/17/2012
 *
 * \author	Bastian Baetz
 */

#ifndef POOLVARIABLE_H_
#define POOLVARIABLE_H_

#include "DataSetIF.h"
#include "PoolEntry.h"
#include "PoolVariableIF.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

template<typename T, uint8_t n_var> class PoolVarList;

/**
 * \brief	This is the access class for non-array data pool entries.
 *
 * \details	To ensure safe usage of the data pool, operation is not done directly on the data pool
 * 			entries, but on local copies. This class provides simple type-safe access to single
 * 			data pool entries (i.e. entries with length = 1).
 * 			The class can be instantiated as read-write and read only.
 * 			It provides a commit-and-roll-back semantic, which means that the variable's value in
 * 			the data pool is not changed until the commit call is executed.
 * 	\tparam	T The template parameter sets the type of the variable. Currently, all plain data types
 * 			are supported, but in principle any type is possible.
 *	\ingroup data_pool
 */
template<typename T>
class PoolVariable: public PoolVariableIF {
	template<typename U, uint8_t n_var> friend class PoolVarList;
protected:
	/**
	 * \brief	To access the correct data pool entry on read and commit calls, the data pool id
	 * 			is stored.
	 */
	uint32_t dataPoolId;
	/**
	 * \brief	The valid information as it was stored in the data pool is copied to this attribute.
	 */
	uint8_t valid;
	/**
	 * \brief	The information whether the class is read-write or read-only is stored here.
	 */
	ReadWriteMode_t readWriteMode;
	/**
	 * \brief	This is a call to read the value from the global data pool.
	 * \details	When executed, this operation tries to fetch the pool entry with matching
	 * 			data pool id from the global data pool and copies the value and the valid
	 * 			information to its local attributes. In case of a failure (wrong type or
	 * 			pool id not found), the variable is set to zero and invalid.
	 * 			The operation does NOT provide any mutual exclusive protection by itself.
	 */
	ReturnValue_t read() {
		PoolEntry<T> *read_out = ::dataPool.getData < T > (dataPoolId, 1);
		if (read_out != NULL) {
			valid = read_out->valid;
			value = *(read_out->address);
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			value = 0;
			valid = false;
			sif::error << "PoolVariable: read of DP Variable 0x" << std::hex
					<< dataPoolId << std::dec << " failed." << std::endl;
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	/**
	 * \brief	The commit call writes back the variable's value to the data pool.
	 * \details	It checks type and size, as well as if the variable is writable. If so,
	 * 			the value is copied and the valid flag is automatically set to "valid".
	 * 			The operation does NOT provide any mutual exclusive protection by itself.
	 *
	 */
	ReturnValue_t commit() {
		PoolEntry<T> *write_back = ::dataPool.getData < T > (dataPoolId, 1);
		if ((write_back != NULL) && (readWriteMode != VAR_READ)) {
			write_back->valid = valid;
			*(write_back->address) = value;
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	/**
	 * Empty ctor for List initialization
	 */
	PoolVariable() :
			dataPoolId(PoolVariableIF::NO_PARAMETER), valid(
					PoolVariableIF::INVALID), readWriteMode(VAR_READ), value(0) {

	}
public:
	/**
	 * \brief	This is the local copy of the data pool entry.
	 * \details	The user can work on this attribute
	 * 			just like he would on a simple local variable.
	 */
	T value;
	/**
	 * \brief	In the constructor, the variable can register itself in a DataSet (if not NULL is
	 * 			passed).
	 * \details	It DOES NOT fetch the current value from the data pool, but sets the value
	 * 			attribute to default (0). The value is fetched within the read() operation.
	 * \param set_id	This is the id in the global data pool this instance of the access class
	 * 					corresponds to.
	 * \param dataSet	The data set in which the variable shall register itself. If NULL,
	 * 					the variable is not registered.
	 * \param setWritable If this flag is set to true, changes in the value attribute can be
	 * 					written back to the data pool, otherwise not.
	 */
	PoolVariable(uint32_t set_id, DataSetIF *dataSet,
			ReadWriteMode_t setReadWriteMode) :
			dataPoolId(set_id), valid(PoolVariableIF::INVALID), readWriteMode(
					setReadWriteMode), value(0) {
		if (dataSet != NULL) {
			dataSet->registerVariable(this);
		}
	}
	/**
	 * Copy ctor to copy classes containing Pool Variables.
	 */
	PoolVariable(const PoolVariable &rhs) :
			dataPoolId(rhs.dataPoolId), valid(rhs.valid), readWriteMode(
					rhs.readWriteMode), value(rhs.value) {
	}

	/**
	 * \brief	The classes destructor is empty.
	 * \details	If commit() was not called, the local value is
	 * 			discarded and not written back to the data pool.
	 */
	~PoolVariable() {

	}
	/**
	 * \brief	This operation returns the data pool id of the variable.
	 */
	uint32_t getDataPoolId() const {
		return dataPoolId;
	}
	/**
	 * This operation sets the data pool id of the variable.
	 * The method is necessary to set id's of data pool member variables with bad initialization.
	 */
	void setDataPoolId(uint32_t poolId) {
		dataPoolId = poolId;
	}
	/**
	 * This method returns if the variable is write-only, read-write or read-only.
	 */
	ReadWriteMode_t getReadWriteMode() const {
		return readWriteMode;
	}
	/**
	 * \brief	With this call, the valid information of the variable is returned.
	 */
	bool isValid() const {
		if (valid)
			return true;
		else
			return false;
	}

	uint8_t getValid() {
		return valid;
	}

	void setValid(uint8_t valid) {
		this->valid = valid;
	}

	operator T() {
		return value;
	}

	operator T() const {
		return value;
	}

	PoolVariable<T>& operator=(T newValue) {
		value = newValue;
		return *this;
	}

	PoolVariable<T>& operator=(PoolVariable<T> newPoolVariable) {
		value = newPoolVariable.value;
		return *this;
	}

	virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size,
			size_t maxSize, Endianness streamEndianness) const override {
		return SerializeAdapter::serialize<T>(&value, buffer, size, maxSize,
				streamEndianness);
	}

	virtual size_t getSerializedSize() const override {
		return SerializeAdapter::getSerializedSize(&value);
	}

	virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
			Endianness streamEndianness) override {
		return SerializeAdapter::deSerialize(&value, buffer, size, streamEndianness);
	}
};

typedef PoolVariable<uint8_t> db_uint8_t;
typedef PoolVariable<uint16_t> db_uint16_t;
typedef PoolVariable<uint32_t> db_uint32_t;
typedef PoolVariable<int8_t> db_int8_t;
typedef PoolVariable<int16_t> db_int16_t;
typedef PoolVariable<int32_t> db_int32_t;
typedef PoolVariable<uint8_t> db_bool_t;
typedef PoolVariable<float> db_float_t;
typedef PoolVariable<double> db_double_t;
//Alternative (but I thing this is not as useful: code duplication, differences too small):

//template <typename T>
//class PoolReader : public PoolVariableIF {
//private:
//	uint32_t parameter_id;
//	uint8_t valid;
//public:
//	T value;
//	PoolReader( uint32_t set_id, DataSetIF* set ) : parameter_id(set_id), valid(false), value(0) {
//		set->registerVariable( this );
//	}
//
//	~PoolReader() {};
//
//	uint8_t commit() {
//			return HasReturnvaluesIF::RETURN_OK;
//	}
//
//	uint8_t read() {
//		PoolEntry<T>* read_out = ::dataPool.getData<T>( parameter_id, 1 );
//		if ( read_out != NULL ) {
//			valid = read_out->valid;
//			value = *(read_out->address);
//			return HasReturnvaluesIF::RETURN_OK;
//		} else {
//			value = 0;
//			valid = false;
//			return CHECKOUT_FAILED;
//		}
//	}
//	uint32_t getParameterId() { return parameter_id; }
//	bool isWritable() { return false; };
//	bool isValid() { if (valid) return true; else return false; }
//};
//
//template <typename T>
//class PoolWriter : public PoolVariableIF {
//private:
//	uint32_t parameter_id;
//public:
//	T value;
//	PoolWriter( uint32_t set_id, DataSetIF* set ) : parameter_id(set_id), value(0) {
//		set->registerVariable( this );
//	}
//
//	~PoolWriter() {};
//
//	uint8_t commit() {
//		PoolEntry<T>* write_back = ::dataPool.getData<T>( parameter_id, 1 );
//		if ( write_back != NULL ) {
//			write_back->valid = true;
//			*(write_back->address) = value;
//			return HasReturnvaluesIF::RETURN_OK;
//		} else {
//			return CHECKOUT_FAILED;
//		}
//	}
//	uint8_t read() {
//		PoolEntry<T>* read_out = ::dataPool.getData<T>( parameter_id, 1 );
//		if ( read_out != NULL ) {
//			value = *(read_out->address);
//			return HasReturnvaluesIF::RETURN_OK;
//		} else {
//			value = 0;
//			return CHECKOUT_FAILED;
//		}
//	}
//	uint32_t getParameterId() { return parameter_id; }
//	bool isWritable() { return true; };
//	bool isValid() { return false; }
//};

#endif /* POOLVARIABLE_H_ */
