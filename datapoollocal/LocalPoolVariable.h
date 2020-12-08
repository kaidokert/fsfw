#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_

#include "LocalPoolObjectBase.h"
#include "HasLocalDataPoolIF.h"
#include "LocalDataPoolManager.h"

#include "../datapool/PoolVariableIF.h"
#include "../datapool/DataSetIF.h"
#include "../objectmanager/ObjectManagerIF.h"
#include "../serialize/SerializeAdapter.h"

/**
 * @brief 	Local Pool Variable class which is used to access the local pools.
 * @details
 * This class is not stored in the map. Instead, it is used to access
 * the pool entries by using a pointer to the map storing the pool
 * entries. It can also be used to organize these pool entries into data sets.
 *
 * @tparam T The template parameter sets the type of the variable. Currently,
 * all plain data types are supported, but in principle  any type is possible.
 * @ingroup data_pool
 */
template<typename T>
class LocalPoolVar: public LocalPoolObjectBase {
public:
	//! Default ctor is forbidden.
	LocalPoolVar() = delete;

	/**
	 * This constructor is used by the data creators to have pool variable
	 * instances which can also be stored in datasets.
	 *
	 * It does not fetch the current value from the data pool, which
	 * has to be done by calling the read() operation.
	 * Datasets can be used to access multiple local pool entries in an
	 * efficient way. A pointer to a dataset can be passed to register
	 * the pool variable in that dataset directly.
	 * @param poolId ID of the local pool entry.
	 * @param hkOwner Pointer of the owner. This will generally be the calling
	 * class itself which passes "this".
	 * @param dataSet The data set in which the variable shall register itself.
	 * If nullptr, the variable is not registered.
	 * @param setReadWriteMode Specify the read-write mode of the pool variable.
	 */
	LocalPoolVar(HasLocalDataPoolIF* hkOwner, lp_id_t poolId,
			DataSetIF* dataSet = nullptr,
			pool_rwm_t setReadWriteMode = pool_rwm_t::VAR_READ_WRITE);

	/**
	 * This constructor is used by data users like controllers to have
	 * access to the local pool variables of data creators by supplying
	 * the respective creator object ID.
	 *
	 * It does not fetch the current value from the data pool, which
	 * has to be done by calling the read() operation.
	 * Datasets can be used to access multiple local pool entries in an
	 * efficient way. A pointer to a dataset can be passed to register
	 * the pool variable in that dataset directly.
	 * @param poolId ID of the local pool entry.
	 * @param hkOwner object ID of the pool owner.
	 * @param dataSet The data set in which the variable shall register itself.
	 * If nullptr, the variable is not registered.
	 * @param setReadWriteMode Specify the read-write mode of the pool variable.
	 *
	 */
	LocalPoolVar(object_id_t poolOwner, lp_id_t poolId,
			DataSetIF* dataSet = nullptr,
			pool_rwm_t setReadWriteMode = pool_rwm_t::VAR_READ_WRITE);
	/**
	 * Variation which takes the global unique identifier of a pool variable.
	 * @param globalPoolId
	 * @param dataSet
	 * @param setReadWriteMode
	 */
	LocalPoolVar(gp_id_t globalPoolId, DataSetIF* dataSet = nullptr,
			pool_rwm_t setReadWriteMode = pool_rwm_t::VAR_READ_WRITE);

	virtual~ LocalPoolVar() {};

	/**
	 * @brief	This is the local copy of the data pool entry.
	 * @details	The user can work on this attribute
	 * 			just like he would on a simple local variable.
	 */
	T value = 0;

	ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
	        SerializeIF::Endianness streamEndianness) const override;
	virtual size_t getSerializedSize() const override;
	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
	        SerializeIF::Endianness streamEndianness) override;

	/**
	 * @brief	This is a call to read the array's values
	 * 			from the global data pool.
	 * @details
	 * When executed, this operation tries to fetch the pool entry with matching
	 * data pool id from the data pool and copies all array values and the valid
	 * information to its local attributes.
	 * In case of a failure (wrong type, size or pool id not found), the
	 * variable is set to zero and invalid.
	 * The read call is protected with a lock.
	 * It is recommended to use DataSets to read and commit multiple variables
	 * at once to avoid the overhead of unnecessary lock und unlock operations.
	 *
	 */
	ReturnValue_t read(dur_millis_t lockTimeout = MutexIF::BLOCKING) override;
	/**
	 * @brief	The commit call copies the array values back to the data pool.
	 * @details
	 * It checks type and size, as well as if the variable is writable. If so,
	 * the value is copied and the local valid flag is written back as well.
	 * The read call is protected with a lock.
	 * It is recommended to use DataSets to read and commit multiple variables
	 * at once to avoid the overhead of unnecessary lock und unlock operations.
	 */
	ReturnValue_t commit(dur_millis_t lockTimeout = MutexIF::BLOCKING) override;


	LocalPoolVar<T> &operator=(const T& newValue);
	LocalPoolVar<T> &operator=(const LocalPoolVar<T>& newPoolVariable);

	//! Explicit type conversion operator. Allows casting the class to
	//! its template type to perform operations on value.
	explicit operator T() const;

	bool operator==(const LocalPoolVar<T>& other) const;
	bool operator==(const T& other) const;

	bool operator!=(const LocalPoolVar<T>& other) const;
	bool operator!=(const T& other) const;

	bool operator<(const LocalPoolVar<T>& other) const;
	bool operator<(const T& other) const;

	bool operator>(const LocalPoolVar<T>& other) const;
	bool operator>(const T& other) const;

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

	// std::ostream is the type for object std::cout
	template <typename U>
	friend std::ostream& operator<< (std::ostream &out,
			const LocalPoolVar<U> &var);

private:
};

#include "LocalPoolVariable.tpp"

template<class T>
using lp_var_t = LocalPoolVar<T>;

using lp_bool_t = LocalPoolVar<uint8_t>;
using lp_uint8_t = LocalPoolVar<uint8_t>;
using lp_uint16_t = LocalPoolVar<uint16_t>;
using lp_uint32_t = LocalPoolVar<uint32_t>;
using lp_uint64_t = LocalPoolVar<uint64_t>;
using lp_int8_t = LocalPoolVar<int8_t>;
using lp_int16_t = LocalPoolVar<int16_t>;
using lp_int32_t = LocalPoolVar<int32_t>;
using lp_int64_t = LocalPoolVar<int64_t>;
using lp_float_t = LocalPoolVar<float>;
using lp_double_t = LocalPoolVar<double>;

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_ */