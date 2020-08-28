#ifndef GLOBALPOOLVECTOR_H_
#define GLOBALPOOLVECTOR_H_

#include "../datapool/DataSetIF.h"
#include "../datapool/PoolEntry.h"
#include "../datapool/PoolVariableIF.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

/**
 * @brief	This is the access class for array-type data pool entries.
 *
 * @details
 * To ensure safe usage of the data pool, operation is not done directly on the
 * data pool entries, but on local copies. This class provides simple type-
 * and length-safe access to vector-style data pool entries (i.e. entries with
 * length > 1). The class can be instantiated as read-write and read only.
 *
 * It provides a commit-and-roll-back semantic, which means that no array
 * entry in the data pool is changed until the commit call is executed.
 * There are two template parameters:
 * @tparam	T
 * This template parameter specifies the data type of an array entry. Currently,
 * all plain data types are supported, but in principle any type is possible.
 * @tparam  vector_size
 * This template parameter specifies the vector size of this entry. Using a
 * template parameter for this is not perfect, but avoids
 * dynamic memory allocation.
 * @ingroup data_pool
 */
template<typename T, uint16_t vectorSize>
class GlobPoolVector: public PoolVariableIF {
public:
	/**
	 * @brief	In the constructor, the variable can register itself in a
	 * 			DataSet (if no nullptr is passed).
	 * @details
	 * It DOES NOT fetch the current value from the data pool, but sets the
	 * value attribute to default (0). The value is fetched within the
	 * read() operation.
	 * @param set_id
	 * This is the id in the global data pool this instance of the access
	 * class corresponds to.
	 * @param dataSet
	 * The data set in which the variable shall register itself. If nullptr,
	 * the variable is not registered.
	 * @param setWritable
	 * If this flag is set to true, changes in the value attribute can be
	 * written back to the data pool, otherwise not.
	 */
	GlobPoolVector(uint32_t set_id, DataSetIF* set,
			ReadWriteMode_t setReadWriteMode);

	/**
	 * @brief	This is the local copy of the data pool entry.
	 * @details	The user can work on this attribute
	 * 			just like he would on a local array of this type.
	 */
	T value[vectorSize];
	/**
	 * @brief	The classes destructor is empty.
	 * @details	If commit() was not called, the local value is
	 * 			discarded and not written back to the data pool.
	 */
	~GlobPoolVector() {};
	/**
	 * @brief	The operation returns the number of array entries
	 * 			in this variable.
	 */
	uint8_t getSize() {
		return vectorSize;
	}
	/**
	 * @brief	This operation returns the data pool id of the variable.
	 */
	uint32_t getDataPoolId() const {
		return dataPoolId;
	}
	/**
	 * @brief This operation sets the data pool id of the variable.
	 * @details
	 * The method is necessary to set id's of data pool member variables
	 * with bad initialization.
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
	 * @brief	With this call, the valid information of the variable is returned.
	 */
	bool isValid() const {
		if (valid != INVALID)
			return true;
		else
			return false;
	}
	void setValid(bool valid) {this->valid = valid;}
	uint8_t getValid() {return valid;}

	T &operator [](int i) {return value[i];}
	const T &operator [](int i) const {return value[i];}

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			size_t max_size, Endianness streamEndianness) const override;
	virtual size_t getSerializedSize() const override;
	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
	        Endianness streamEndianness) override;

	/**
	 * @brief	This is a call to read the array's values
	 * 			from the global data pool.
	 * @details
	 * When executed, this operation tries to fetch the pool entry with matching
	 * data pool id from the global data pool and copies all array values
	 * and the valid information to its local attributes.
	 * In case of a failure (wrong type, size or pool id not found), the
	 * variable is set to zero and invalid.
	 * The read call is protected by a lock of the global data pool.
	 * It is recommended to use DataSets to read and commit multiple variables
	 * at once to avoid the overhead of unnecessary lock und unlock operations.
	 */
	ReturnValue_t read(uint32_t lockTimeout = MutexIF::BLOCKING) override;
	/**
	 * @brief	The commit call copies the array values back to the data pool.
	 * @details
	 * It checks type and size, as well as if the variable is writable. If so,
	 * the value is copied and the valid flag is automatically set to "valid".
	 * The commit call is protected by a lock of the global data pool.
	 * It is recommended to use DataSets to read and commit multiple variables
	 * at once to avoid the overhead of unnecessary lock und unlock operations.
	 */
	ReturnValue_t commit(uint32_t lockTimeout = MutexIF::BLOCKING) override;

protected:
	/**
	 * @brief	Like #read, but without a lock protection of the global pool.
	 * @details
	 * The operation does NOT provide any mutual exclusive protection by itself.
	 * This can be used if the lock is handled externally to avoid the overhead
	 * of consecutive lock und unlock operations.
	 * Declared protected to discourage free public usage.
	 */
	ReturnValue_t readWithoutLock() override;
	/**
	 * @brief	Like #commit, but without a lock protection of the global pool.
	 * @details
	 * The operation does NOT provide any mutual exclusive protection by itself.
	 * This can be used if the lock is handled externally to avoid the overhead
	 * of consecutive lock und unlock operations.
	 * Declared protected to discourage free public usage.
	 */
	ReturnValue_t commitWithoutLock() override;

private:
	/**
	 * @brief	To access the correct data pool entry on read and commit calls,
	 * 			the data pool id is stored.
	 */
	uint32_t dataPoolId;
	/**
	 * @brief	The valid information as it was stored in the data pool
	 * 			is copied to this attribute.
	 */
	uint8_t valid;
	/**
	 * @brief	The information whether the class is read-write or
	 * 			read-only is stored here.
	 */
	ReadWriteMode_t readWriteMode;
};

#include "../datapoolglob/GlobalPoolVector.tpp"

template<typename T, uint16_t vectorSize>
using gp_vec_t = GlobPoolVector<T, vectorSize>;

#endif /* POOLVECTOR_H_ */
