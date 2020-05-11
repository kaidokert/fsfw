#ifndef POOLENTRY_H_
#define POOLENTRY_H_

#include <framework/datapool/PoolEntryIF.h>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <type_traits>
/**
 * @brief	This is a small helper class that defines a single data pool entry.
 * @details
 * The helper is used to store all information together with the data as a
 * single data pool entry.The content's type is defined by the template argument.
 * It is prepared for use with plain old data types, but may be extended to
 * complex types if necessary. It can be initialized with a certain value,
 * size and validity flag. It holds a pointer to the real data and offers
 * methods to access this data and to acquire additional information
 * (such as validity and array/byte size). It is NOT intended to be used
 * outside the DataPool class.
 *
 * @ingroup data_pool
 *
 */
template <typename T>
class PoolEntry : public PoolEntryIF {
public:
	static_assert(not std::is_same<T, bool>::value,
				"Do not use boolean for the PoolEntry type, use uint8_t instead!"
				"Warum? Darum :-)");
	/**
	 * @brief	In the classe's constructor, space is allocated on the heap and
	 * 			potential init values are copied to that space.
	 * @param initValue	Initializer list with values to initialize with
	 * @param set_length	Defines the array length of this entry.
	 * @param set_valid		Sets the initialization flag.
	 * 						It is invalid (0) by default.
	 */
	PoolEntry(std::initializer_list<T> initValue = {}, uint8_t set_length = 1,
			uint8_t set_valid = 0);
	/**
	 * @brief	In the classe's constructor, space is allocated on the heap and
	 * 			potential init values are copied to that space.
	 * @param initValue	A pointer to the single value or array that holds
	 *  the init value. With the default value (nullptr), the entry is
	 *  initalized with all 0.
	 * @param set_length	Defines the array length of this entry.
	 * @param set_valid		Sets the initialization flag. It is invalid (0) by default.
	 */
	PoolEntry(T* initValue = nullptr, uint8_t set_length = 1, uint8_t set_valid = 0);

	/**
	 * @brief	The allocated memory for the variable is freed in the destructor.
	 * @details	As the data pool is global, this dtor is only called on program exit.
	 * PoolEntries shall never be copied, as a copy might delete the variable on the heap.
	 */
	~PoolEntry();
	/**
	 * @brief	This is the address pointing to the allocated memory.
	 */
	T* address;
	/**
	 * @brief	This attribute stores the length information.
	 */
	uint8_t length;
	/**
	 * @brief	Here, the validity information for a variable is stored.
	 * 			Every entry (single variable or vector) has one valid flag.
	 */
	uint8_t valid;
	/**
	 * @brief	getSize returns the array size of the entry.
	 * @details	A single parameter has size 1.
	 */
	uint8_t getSize();
	/**
	 * @brief	This operation returns the size in bytes.
	 * @details	The size is calculated by sizeof(type) * array_size.
	 */
	uint16_t getByteSize();
	/**
	 * @brief	This operation returns a the address pointer casted to void*.
	 */
	void* getRawData();
	/**
	 * @brief	This method allows to set the valid information of the pool entry.
	 */
	void setValid( uint8_t isValid );
	/**
	 * @brief	This method allows to get the valid information of the pool entry.
	 */
	uint8_t getValid();
	/**
	 * @brief	This is a debug method that prints all values and the valid
	 * 			information to the screen. It prints all array entries in a row.
	 */
	void print();

	Type getType();
};

#endif /* POOLENTRY_H_ */
