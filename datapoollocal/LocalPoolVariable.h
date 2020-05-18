#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_

#include <framework/datapool/PoolVariableIF.h>
#include <framework/datapool/DataSetIF.h>

#include <framework/housekeeping/HousekeepingManager.h>

#include <map>
/**
 * @brief	This is the access class for non-array local data pool entries.
 *
 * @details
 *
 * 	@tparam	T The template parameter sets the type of the variable.
 * 	Currently, all plain data types are supported, but in principle
 * 	any type is possible.
 *	@ingroup data_pool
 */

/**
 * @brief 	Local Pool Variable class which is used to access the local pools.
 * @details This class is not stored in the map. Instead, it is used to access
 * 			the pool entries by using a pointer to the map storing the pool
 * 			entries. It can also be used to organize these pool entries
 * 			into data sets.
 * @tparam T
 */
template<typename T>
class LocalPoolVar: public PoolVariableIF, HasReturnvaluesIF {
public:
	static constexpr lp_id_t INVALID_POOL_ID = 0xFFFFFFFF;

	/**
	 * This constructor is used by the data creators to have pool variable
	 * instances which can also be stored in datasets.
	 * @param set_id
	 * @param setReadWriteMode
	 * @param localPoolMap
	 * @param dataSet
	 */
	LocalPoolVar(lp_id_t poolId, HasHkPoolParametersIF* hkOwner,
			pool_rwm_t setReadWriteMode, DataSetIF* dataSet = nullptr);

	/**
	 * This constructor is used by data users like controllers to have
	 * access to the local pool variables of data creators by supplying
	 * the respective creator object ID.
	 * @param poolId
	 * @param poolOwner
	 * @param setReadWriteMode
	 * @param dataSet
	 */
	LocalPoolVar(lp_id_t poolId, object_id_t poolOwner,
			pool_rwm_t setReadWriteMode, DataSetIF* dataSet = nullptr);

	virtual~ LocalPoolVar() {};

	/**
	 * @brief	This is the local copy of the data pool entry.
	 * @details	The user can work on this attribute
	 * 			just like he would on a simple local variable.
	 */
	T value = 0;

	ReturnValue_t commit() override;
	ReturnValue_t read() override;
	pool_rwm_t getReadWriteMode() const override;
	uint32_t getDataPoolId() const override;
	bool isValid() const override;
	void setValid(uint8_t validity) override;

	ReturnValue_t serialize(uint8_t** buffer, size_t* size,
				const size_t max_size, bool bigEndian) const override;
	virtual size_t getSerializedSize() const override;
	virtual ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size,
				bool bigEndian) override;
private:
	lp_id_t localPoolId = INVALID_POOL_ID;
	pool_rwm_t readWriteMode = pool_rwm_t::VAR_READ_WRITE;
	bool valid = false;

	bool objectValid = true;
	//! Pointer to the class which manages the HK pool.
	HousekeepingManager* hkManager;
};

#include <framework/datapoollocal/LocalPoolVariable.tpp>
template<class T>
using lp_variable = LocalPoolVar<T>;

using lp_bool_t = LocalPoolVar<bool>;
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

#endif
