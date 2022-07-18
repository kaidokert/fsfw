#ifndef FSFW_DATAPOOL_POOLENTRY_H_
#define FSFW_DATAPOOL_POOLENTRY_H_

#include <cstddef>
#include <initializer_list>
#include <type_traits>

#include "PoolEntryIF.h"

/**
 * @brief	This is a small helper class that defines a single data pool entry.
 * @details
 * The helper is used to store all information together with the data as a
 * single data pool entry. The content's type is defined by the template
 * argument.
 *
 * It is prepared for use with plain old data types, but may be
 * extended to complex types if necessary. It can be initialized with a
 * certain value, size and validity flag.
 *
 * It holds a pointer to the real data and offers methods to access this data
 * and to acquire additional information (such as validity and array/byte size).
 * It is NOT intended to be used outside DataPool implementations as it performs
 * dynamic memory allocation.
 *
 * @ingroup data_pool
 */
template <typename T>
class PoolEntry : public PoolEntryIF {
 public:
  static_assert(not std::is_same<T, bool>::value,
                "Do not use boolean for the PoolEntry type, use uint8_t "
                "instead! The ECSS standard defines a boolean as a one bit "
                "field. Therefore it is preferred to store a boolean as an "
                "uint8_t");

  PoolEntry(uint8_t len = 1, bool setValid = false);

  /**
   * @brief	In the classe's constructor, space is allocated on the heap and
   * 			potential initialization values are copied to that space.
   * @details
   * Not passing any arguments will initialize an non-array pool entry
   * with an initial invalid state and the value 0.
   * Please note that if an initializer list is passed, the length of the
   * initializer list needs to be correct for vector entries because
   * required allocated space will be deduced from the initializer list length
   * and the pool entry type.
   * @param initValue
   * Initializer list with values to initialize with, for example {0, 0} to
   * initialize the a pool entry of a vector with two entries to 0.
   * @param setValid
   * Sets the initialization flag. It is invalid by default.
   */
  PoolEntry(std::initializer_list<T> initValue, bool setValid = false);

  /**
   * @brief	In the classe's constructor, space is allocated on the heap and
   * 			potential init values are copied to that space.
   * @param initValue
   * A pointer to the single value or array that holds the init value.
   * With the default value (nullptr), the entry is initalized with all 0.
   * @param setLength
   * Defines the array length of this entry.
   * @param setValid
   * Sets the initialization flag. It is invalid by default.
   */
  PoolEntry(const T* initValue, uint8_t setLength = 1, bool setValid = false);

  //! Explicitely deleted copy ctor, copying is not allowed.
  PoolEntry(const PoolEntry&) = delete;
  //! Explicitely deleted copy assignment, copying is not allowed.
  PoolEntry& operator=(const PoolEntry&) = delete;

  /**
   * @brief	The allocated memory for the variable is freed
   * 			in the destructor.
   * @details
   * As the data pool is global, this dtor is only called on program exit.
   * PoolEntries shall never be copied, as a copy might delete the variable
   * on the heap.
   */
  ~PoolEntry();

  /**
   * Return typed pointer to start of data.
   * @return
   */
  T* getDataPtr();

  /**
   * @brief	getSize returns the array size of the entry.
   * @details
   * For non-array pool entries return type size, for vector entries
   * return type size times the number of entries.
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
   * @brief	This method allows to set the valid information
   * 			of the pool entry.
   */
  void setValid(bool isValid);
  /**
   * @brief	This method allows to get the valid information
   * 			of the pool entry.
   */
  bool getValid();
  /**
   * @brief	This is a debug method that prints all values and the valid
   * 			information to the screen. It prints all array entries in a row.
   */
  void print();
  Type getType();

 private:
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
   * @brief	This is the address pointing to the allocated memory.
   */
  T* address;
};

#endif /* FSFW_DATAPOOL_POOLENTRY_H_ */
