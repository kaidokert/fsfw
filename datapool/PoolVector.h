/*
 * \file	PoolVector.h
 *
 * \brief	This file contains the PoolVector class, the header only class to handle data pool vectors.
 *
 * \date	10/23/2012
 *
 * \author	Bastian Baetz
 */

#ifndef POOLVECTOR_H_
#define POOLVECTOR_H_

#include "DataSetIF.h"
#include "PoolEntry.h"
#include "PoolVariableIF.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

/**
 * \brief	This is the access class for array-type data pool entries.
 *
 * \details	To ensure safe usage of the data pool, operation is not done directly on the data pool
 * 			entries, but on local copies. This class provides simple type- and length-safe access
 * 			to vector-style data pool entries (i.e. entries with length > 1).
 * 			The class can be instantiated as read-write and read only.
 * 			It provides a commit-and-roll-back semantic, which means that no array entry in
 * 			the data pool is changed until the commit call is executed.
 * 			There are two template parameters:
 * 	\tparam	T This template parameter specifies the data type of an array entry. Currently, all
 * 			plain data types are supported, but in principle any type is possible.
 * 	\tparam vector_size	This template parameter specifies the vector size of this entry.
 * 			Using a template parameter for this is not perfect, but avoids dynamic memory allocation.
 *	\ingroup data_pool
 */
template<typename T, uint16_t vector_size>
class PoolVector: public PoolVariableIF {
private:
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

protected:
	/**
	 * \brief	This is a call to read the array's values from the global data pool.
	 * \details	When executed, this operation tries to fetch the pool entry with matching
	 * 			data pool id from the global data pool and copies all array values and the valid
	 * 			information to its local attributes. In case of a failure (wrong type, size or
	 * 			pool id not found), the variable is set to zero and invalid.
	 * 			The operation does NOT provide any mutual exclusive protection by itself.
	 */
	ReturnValue_t read() {
		PoolEntry<T>* read_out = ::dataPool.getData<T>(this->dataPoolId,
				vector_size);
		if (read_out != NULL) {
			this->valid = read_out->valid;
			memcpy(this->value, read_out->address, read_out->getByteSize());

			return HasReturnvaluesIF::RETURN_OK;

		} else {
			memset(this->value, 0, vector_size * sizeof(T));
			sif::error << "PoolVector: read of DP Variable 0x" << std::hex
					<< dataPoolId << std::dec << " failed." << std::endl;
			this->valid = INVALID;
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	/**
	 * \brief	The commit call copies the array values back to the data pool.
	 * \details	It checks type and size, as well as if the variable is writable. If so,
	 * 			the value is copied and the valid flag is automatically set to "valid".
	 * 			The operation does NOT provide any mutual exclusive protection by itself.
	 *
	 */
	ReturnValue_t commit() {
		PoolEntry<T>* write_back = ::dataPool.getData<T>(this->dataPoolId,
				vector_size);
		if ((write_back != NULL) && (this->readWriteMode != VAR_READ)) {
			write_back->valid = valid;
			memcpy(write_back->address, this->value, write_back->getByteSize());
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
public:
	/**
	 * \brief	This is the local copy of the data pool entry.
	 * \detials	The user can work on this attribute
	 * 			just like he would on a local array of this type.
	 */
	T value[vector_size];
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
	PoolVector(uint32_t set_id, DataSetIF* set,
			ReadWriteMode_t setReadWriteMode) :
			dataPoolId(set_id), valid(false), readWriteMode(setReadWriteMode) {
		memset(this->value, 0, vector_size * sizeof(T));
		if (set != NULL) {
			set->registerVariable(this);
		}
	}
	/**
	 * Copy ctor to copy classes containing Pool Variables.
	 */
//	PoolVector(const PoolVector& rhs) {
//		PoolVector<T, vector_size> temp(rhs.dataPoolId, rhs.)
//		memcpy(value, rhs.value, sizeof(T)*vector_size);
//	}
	/**
	 * \brief	The classes destructor is empty.
	 * \details	If commit() was not called, the local value is
	 * 			discarded and not written back to the data pool.
	 */
	~PoolVector() {
	}
	;
	/**
	 * \brief	The operation returns the number of array entries in this variable.
	 */
	uint8_t getSize() {
		return vector_size;
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
	;
	/**
	 * \brief	With this call, the valid information of the variable is returned.
	 */
	bool isValid() const {
		if (valid != INVALID)
			return true;
		else
			return false;
	}

	void setValid(uint8_t valid) {
		this->valid = valid;
	}

	uint8_t getValid() {
		return valid;
	}

	T &operator [](int i) {
		return value[i];
	}

	const T &operator [](int i) const {
		return value[i];
	}

	PoolVector<T, vector_size> &operator=(
			PoolVector<T, vector_size> newPoolVector) {

		for (uint16_t i = 0; i < vector_size; i++) {
			this->value[i] = newPoolVector.value[i];
		}
		return *this;
	}

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t maxSize, Endianness streamEndianness) const {
		uint16_t i;
		ReturnValue_t result;
		for (i = 0; i < vector_size; i++) {
			result = SerializeAdapter::serialize(&(value[i]), buffer, size,
					maxSize, streamEndianness);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
		}
		return result;
	}

	virtual size_t getSerializedSize() const {
		return vector_size * SerializeAdapter::getSerializedSize(value);
	}

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
			Endianness streamEndianness) {
		uint16_t i;
		ReturnValue_t result;
		for (i = 0; i < vector_size; i++) {
			result = SerializeAdapter::deSerialize(&(value[i]), buffer, size,
					streamEndianness);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				return result;
			}
		}
		return result;
	}
};

#endif /* POOLVECTOR_H_ */
