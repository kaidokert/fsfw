#ifndef POOLENTRYIF_H_
#define POOLENTRYIF_H_

#include <framework/globalfunctions/Type.h>
#include <cstdint>

/**
 * @brief	This interface defines the access possibilities to a
 * 			single data pool entry.
 * @details
 * The interface provides methods to determine the size and the validity
 * information of a value. It also defines a method to receive a pointer to
 * the raw data content. It is mainly used by DataPool itself, but also as a
 * return pointer.
 * @author 	Bastian Baetz
 * @ingroup data_pool
 */
class PoolEntryIF {
public:
	/**
	 * @brief	This is an empty virtual destructor,
	 * 			as it is proposed for C++ interfaces.
	 */
	virtual ~PoolEntryIF() {}

	/**
	 * @brief	getSize returns the array size of the entry.
	 * 			A single variable parameter has size 1.
	 */
	virtual uint8_t getSize() = 0;

	/**
	 * @brief	This operation returns the size in bytes, which is calculated by
	 * 			sizeof(type) * array_size.
	 */
	virtual uint16_t getByteSize() = 0;

	/**
	 * @brief	This operation returns a the address pointer casted to void*.
	 */
	virtual void* getRawData() = 0;

	/**
	 * @brief	This method allows to set the valid information of the pool entry.
	 */
	virtual void setValid(uint8_t isValid) = 0;

	/**
	 * @brief	This method allows to set the valid information of the pool entry.
	 */
	virtual uint8_t getValid() = 0;

	/**
	 * @brief	This is a debug method that prints all values and the valid
	 * 			information to the screen. It prints all array entries in a row.
	 */
	virtual void print() = 0;

	/**
	 * @brief 	Returns the type of the entry.
	 */
	virtual Type getType() = 0;
};

#endif /* POOLENTRYIF_H_ */
