#ifndef GLOBALPOOLVARIABLE_H_
#define GLOBALPOOLVARIABLE_H_

#include "../datapool/DataSetIF.h"
#include "../datapoolglob/GlobalDataPool.h"
#include "../datapool/PoolVariableIF.h"
#include "../datapool/PoolEntry.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

template<typename T, uint8_t n_var> class PoolVarList;


/**
 * @brief	This is the access class for non-array data pool entries.
 *
 * @details
 * To ensure safe usage of the data pool, operation is not done directly
 * on the data pool entries, but on local copies. This class provides simple
 * type-safe access to single data pool entries (i.e. entries with length = 1).
 * The class can be instantiated as read-write and read only.
 * It provides a commit-and-roll-back semantic, which means that the
 * variable's value in the data pool is not changed until the
 * commit call is executed.
 * 	@tparam	T The template parameter sets the type of the variable.
 * 	Currently, all plain data types are supported, but in principle
 * 	any type is possible.
 *	@ingroup data_pool
 */
template<typename T>
class GlobPoolVar: public PoolVariableIF {
	template<typename U, uint8_t n_var> friend class PoolVarList;
	static_assert(not std::is_same<T, bool>::value,
			"Do not use boolean for the PoolEntry type, use uint8_t instead!"
			"There is no boolean type in CCSDS.");
public:
	/**
	 * @brief	In the constructor, the variable can register itself in a
	 * 			DataSet (if nullptr is not passed).
	 * @details
	 * It DOES NOT fetch the current value from the data pool, but
	 * sets the value attribute to default (0).
	 * The value is fetched within the read() operation.
	 * @param set_id	This is the id in the global data pool
	 * 					this instance of the access class corresponds to.
	 * @param dataSet	The data set in which the variable shall register
	 * 					itself. If NULL, the variable is not registered.
	 * @param setWritable If this flag is set to true, changes in the value
	 * 			attribute can be written back to the data pool, otherwise not.
	 */
	GlobPoolVar(uint32_t set_id, DataSetIF* dataSet,
			ReadWriteMode_t setReadWriteMode);

	/**
	 * @brief	This is the local copy of the data pool entry.
	 * @details	The user can work on this attribute
	 * 			just like he would on a simple local variable.
	 */
	T value = 0;

	/**
	 * @brief 	Copy ctor to copy classes containing Pool Variables.
	 * 			(Robin): This only copies member variables, which is done
	 * 			by the default copy ctor. maybe we can ommit this ctor?
	 */
	GlobPoolVar(const GlobPoolVar& rhs);

	/**
	 * @brief	The classes destructor is empty.
	 * @details	If commit() was not called, the local value is
	 * 			discarded and not written back to the data pool.
	 */
	~GlobPoolVar() {}

	/**
	 * @brief	This is a call to read the value from the global data pool.
	 * @details
	 * When executed, this operation tries to fetch the pool entry with matching
	 * data pool id from the global data pool and copies the value and the valid
	 * information to its local attributes. In case of a failure (wrong type or
	 * pool id not found), the variable is set to zero and invalid.
	 * The read call is protected with a lock.
	 * It is recommended to use DataSets to read and commit multiple variables
	 * at once to avoid the overhead of unnecessary lock und unlock operations.
	 */
	ReturnValue_t read(uint32_t lockTimeout) override;
	/**
	 * @brief	The commit call writes back the variable's value to the data pool.
	 * @details
	 * It checks type and size, as well as if the variable is writable. If so,
	 * the value is copied and the valid flag is automatically set to "valid".
	 * The operation does NOT provide any mutual exclusive protection by itself.
	 * The commit call is protected with a lock.
	 * It is recommended to use DataSets to read and commit multiple variables
	 * at once to avoid the overhead of unnecessary lock und unlock operations.
	 */
	ReturnValue_t commit(uint32_t lockTimeout) override;

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
	/**
	 * @brief	To access the correct data pool entry on read and commit calls,
	 * 			the data pool is stored.
	 */
	uint32_t dataPoolId;

	/**
	 * @brief	The valid information as it was stored in the data pool is
	 * 			copied to this attribute.
	 */
	uint8_t valid;

	/**
	 * @brief	The information whether the class is read-write or read-only
	 * 			is stored here.
	 */
	pool_rwm_t readWriteMode;

	/**
	 * Empty ctor for List initialization
	 */
	GlobPoolVar();
public:
	/**
	 * \brief	This operation returns the data pool id of the variable.
	 */
	uint32_t getDataPoolId() const override;

	/**
	 * This method returns if the variable is write-only, read-write or read-only.
	 */
	ReadWriteMode_t getReadWriteMode() const override;
	/**
	 * This operation sets the data pool id of the variable.
	 * The method is necessary to set id's of data pool member variables with bad initialization.
	 */
	void setDataPoolId(uint32_t poolId);

	/**
	 * \brief	With this call, the valid information of the variable is returned.
	 */
	bool isValid() const override;

	uint8_t getValid();

	void setValid(bool valid) override;

	operator T() {
		return value;
	}

	operator T() const {
		return value;
	}

	GlobPoolVar<T> &operator=(T newValue) {
		value = newValue;
		return *this;
	}

	GlobPoolVar<T> &operator=(GlobPoolVar<T> newPoolVariable) {
		value = newPoolVariable.value;
		return *this;
	}

	virtual ReturnValue_t serialize(uint8_t** buffer, size_t* size,
			const size_t max_size,
			SerializeIF::Endianness streamEndianness) const override {
		return SerializeAdapter::serialize(&value, buffer, size, max_size,
				streamEndianness);
	}

	virtual size_t getSerializedSize() const {
		return SerializeAdapter::getSerializedSize(&value);
	}

	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
	        SerializeIF::Endianness streamEndianness) {
		return SerializeAdapter::deSerialize(&value, buffer, size,
		        streamEndianness);
	}
};

#include "../datapoolglob/GlobalPoolVariable.tpp"

typedef GlobPoolVar<uint8_t> gp_bool_t;
typedef GlobPoolVar<uint8_t> gp_uint8_t;
typedef GlobPoolVar<uint16_t> gp_uint16_t;
typedef GlobPoolVar<uint32_t> gp_uint32_t;
typedef GlobPoolVar<int8_t> gp_int8_t;
typedef GlobPoolVar<int16_t> gp_int16_t;
typedef GlobPoolVar<int32_t> gp_int32_t;
typedef GlobPoolVar<float> gp_float_t;
typedef GlobPoolVar<double> gp_double_t;

#endif /* POOLVARIABLE_H_ */
